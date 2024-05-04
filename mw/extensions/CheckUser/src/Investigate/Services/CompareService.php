<?php

namespace MediaWiki\CheckUser\Investigate\Services;

use LogicException;
use MediaWiki\Config\ServiceOptions;
use MediaWiki\User\UserIdentityLookup;
use Wikimedia\Rdbms\IConnectionProvider;
use Wikimedia\Rdbms\IDatabase;
use Wikimedia\Rdbms\SelectQueryBuilder;
use Wikimedia\Rdbms\Subquery;

class CompareService extends ChangeService {
	private IConnectionProvider $dbProvider;

	/**
	 * @internal For use by ServiceWiring
	 */
	public const CONSTRUCTOR_OPTIONS = [
		'CheckUserInvestigateMaximumRowCount',
	];

	/** @var int */
	private $limit;

	/**
	 * @param ServiceOptions $options
	 * @param IConnectionProvider $dbProvider
	 * @param UserIdentityLookup $userIdentityLookup
	 */
	public function __construct(
		ServiceOptions $options,
		IConnectionProvider $dbProvider,
		UserIdentityLookup $userIdentityLookup
	) {
		parent::__construct(
			$dbProvider->getReplicaDatabase(),
			$dbProvider->getReplicaDatabase(),
			$userIdentityLookup
		);

		$this->dbProvider = $dbProvider;
		$options->assertRequiredOptions( self::CONSTRUCTOR_OPTIONS );
		$this->limit = $options->get( 'CheckUserInvestigateMaximumRowCount' );
	}

	/**
	 * Get edits made from an ip
	 *
	 * @param string $ipHex
	 * @param string|null $excludeUser
	 * @return int
	 */
	public function getTotalEditsFromIp(
		string $ipHex,
		string $excludeUser = null
	): int {
		$queryBuilder = $this->dbProvider->getReplicaDatabase()->newSelectQueryBuilder()
			->select( 'cuc_id' )
			->from( 'cu_changes' )
			->join( 'actor', null, 'actor_id=cuc_actor' )
			->where( [
				'cuc_ip_hex' => $ipHex,
				'cuc_type' => [ RC_EDIT, RC_NEW ],
			] )
			->limit( $this->limit )
			->caller( __METHOD__ );

		if ( $excludeUser ) {
			$queryBuilder->where(
				'actor_name != ' . $this->dbQuoter->addQuotes( $excludeUser )
			);
		}

		return $queryBuilder->fetchRowCount();
	}

	/**
	 * Get the compare query info
	 *
	 * @param string[] $targets
	 * @param string[] $excludeTargets
	 * @param string $start
	 * @return array
	 */
	public function getQueryInfo( array $targets, array $excludeTargets, string $start ): array {
		$dbr = $this->dbProvider->getReplicaDatabase();

		if ( $targets === [] ) {
			throw new LogicException( 'Cannot get query info when $targets is empty.' );
		}
		$limit = (int)( $this->limit / count( $targets ) );

		$sqlText = [];
		foreach ( $targets as $target ) {
			$conds = $this->buildCondsForSingleTarget( $target, $excludeTargets, $start );
			if ( $conds !== null ) {
				$queryBuilder = $dbr->newSelectQueryBuilder()
					->select( [
						'cuc_id',
						'cuc_user' => 'cuc_user_actor.actor_user',
						'cuc_user_text' => 'cuc_user_actor.actor_name',
						'cuc_ip',
						'cuc_ip_hex',
						'cuc_agent',
						'cuc_timestamp',
					] )
					->from( 'cu_changes' )
					->join( 'actor', 'cuc_user_actor', 'cuc_user_actor.actor_id=cuc_actor' )
					->where( $conds )
					->caller( __METHOD__ );
				if ( $dbr->unionSupportsOrderAndLimit() ) {
					$queryBuilder->orderBy( 'cuc_timestamp', SelectQueryBuilder::SORT_DESC )
						->limit( $limit );
				}
				$sqlText[] = $queryBuilder->getSQL();
			}
		}

		$derivedTable = $dbr->unionQueries( $sqlText, IDatabase::UNION_DISTINCT );

		return [
			'tables' => [ 'a' => new Subquery( $derivedTable ) ],
			'fields' => [
				'cuc_user' => 'a.cuc_user',
				'cuc_user_text' => 'a.cuc_user_text',
				'cuc_ip' => 'a.cuc_ip',
				'cuc_ip_hex' => 'a.cuc_ip_hex',
				'cuc_agent' => 'a.cuc_agent',
				'first_edit' => 'MIN(a.cuc_timestamp)',
				'last_edit' => 'MAX(a.cuc_timestamp)',
				'total_edits' => 'count(*)',
			],
			'options' => [
				'GROUP BY' => [
					'cuc_user',
					'cuc_user_text',
					'cuc_ip',
					'cuc_ip_hex',
					'cuc_agent',
				],
			],
		];
	}

	/**
	 * Get the query info for a single target.
	 *
	 * For the main investigation, this is used in a subquery that contributes to a derived
	 * table, used by getQueryInfo.
	 *
	 * For a limit check, this is used to build a query that is used to check whether the number of results for
	 * the target exceed the limit-per-target in getQueryInfo.
	 *
	 * @param string $target
	 * @param string[] $excludeTargets
	 * @param string $start
	 * @return array|null Return null for invalid target
	 */
	private function buildCondsForSingleTarget(
		string $target,
		array $excludeTargets,
		string $start
	): ?array {
		$conds = $this->buildTargetConds( $target );
		if ( $conds === [] ) {
			return null;
		}

		$conds = array_merge(
			$conds,
			$this->buildExcludeTargetsConds( $excludeTargets ),
			$this->buildStartConds( $start )
		);

		$conds['cuc_type'] = [ RC_EDIT, RC_NEW, RC_LOG ];

		return $conds;
	}

	/**
	 * We set a maximum number of rows in cu_changes to be grouped in the Compare table query,
	 * for performance reasons (see ::getQueryInfo). We share these uniformly between the targets,
	 * so the maximum number of rows per target is the limit divided by the number of targets.
	 *
	 * @param array $targets
	 * @return int
	 */
	private function getLimitPerTarget( array $targets ) {
		return (int)( $this->limit / count( $targets ) );
	}

	/**
	 * Check if we have incomplete data for any of the targets.
	 *
	 * @param string[] $targets
	 * @param string[] $excludeTargets
	 * @param string $start
	 * @return string[]
	 */
	public function getTargetsOverLimit(
		array $targets,
		array $excludeTargets,
		string $start
	): array {
		if ( $targets === [] ) {
			return $targets;
		}

		$dbr = $this->dbProvider->getReplicaDatabase();

		// If the database does not support order and limit on a UNION
		// then none of the targets can be over the limit.
		if ( !$dbr->unionSupportsOrderAndLimit() ) {
			return [];
		}

		$targetsOverLimit = [];
		$offset = $this->getLimitPerTarget( $targets );

		foreach ( $targets as $target ) {
			$conds = $this->buildCondsForSingleTarget( $target, $excludeTargets, $start );
			if ( $conds !== null ) {
				$limitCheck = $dbr->newSelectQueryBuilder()
					->select( 'cuc_id' )
					->from( 'cu_changes' )
					->join( 'actor', 'cuc_user_actor', 'cuc_user_actor.actor_id=cuc_actor' )
					->where( $conds )
					->offset( $offset )
					->limit( 1 )
					->caller( __METHOD__ );
				if ( $limitCheck->fetchRowCount() > 0 ) {
					$targetsOverLimit[] = $target;
				}
			}
		}

		return $targetsOverLimit;
	}
}
