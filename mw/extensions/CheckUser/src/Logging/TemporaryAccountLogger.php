<?php

namespace MediaWiki\CheckUser\Logging;

use ManualLogEntry;
use MediaWiki\Title\Title;
use MediaWiki\User\ActorStore;
use MediaWiki\User\UserIdentity;
use Psr\Log\LoggerInterface;
use Wikimedia\Assert\Assert;
use Wikimedia\Assert\ParameterAssertionException;
use Wikimedia\Rdbms\DBError;
use Wikimedia\Rdbms\IDatabase;

/**
 * Defines the API for the component responsible for logging the following interactions:
 *
 * - A user views IP addresses for a temporary account
 * - A user enables temporary account IP viewing
 * - A user disables temporary account IP viewing
 *
 * All the above interactions will be logged to the `logging` table with a log type
 * `checkuser-temporary-account`.
 */
class TemporaryAccountLogger {
	/**
	 * Represents a user (the performer) viewing IP addresses for a temporary account.
	 *
	 * @var string
	 */
	public const ACTION_VIEW_IPS = 'view-ips';

	/**
	 * Represents a user enabling or disabling their own access to view IPs
	 *
	 * @var string
	 */
	public const ACTION_CHANGE_ACCESS = 'change-access';

	/**
	 * @var string
	 */
	public const ACTION_ACCESS_ENABLED = 'enable';

	/**
	 * @var string
	 */
	public const ACTION_ACCESS_DISABLED = 'disable';

	/**
	 * @var string
	 */
	public const LOG_TYPE = 'checkuser-temporary-account';

	private ActorStore $actorStore;
	private LoggerInterface $logger;
	private IDatabase $dbw;

	private int $delay;

	/**
	 * @param ActorStore $actorStore
	 * @param LoggerInterface $logger
	 * @param IDatabase $dbw
	 * @param int $delay The number of seconds after which a duplicate log entry can be
	 *  created for a debounced log
	 * @throws ParameterAssertionException
	 */
	public function __construct(
		ActorStore $actorStore,
		LoggerInterface $logger,
		IDatabase $dbw,
		int $delay
	) {
		Assert::parameter( $delay > 0, 'delay', 'delay must be positive' );

		$this->actorStore = $actorStore;
		$this->logger = $logger;
		$this->dbw = $dbw;
		$this->delay = $delay;
	}

	/**
	 * Logs the user (the performer) viewing IP addresses for a temporary account.
	 *
	 * @param UserIdentity $performer
	 * @param string $tempUser
	 * @param int $timestamp
	 */
	public function logViewIPs( UserIdentity $performer, string $tempUser, int $timestamp ): void {
		$this->debouncedLog( $performer, $tempUser, self::ACTION_VIEW_IPS, $timestamp );
	}

	/**
	 * Log when the user enables their own access
	 *
	 * @param UserIdentity $performer
	 */
	public function logAccessEnabled( UserIdentity $performer ): void {
		$params = [
			'4::changeType' => self::ACTION_ACCESS_ENABLED,
		];
		$this->log( $performer, $performer->getName(), self::ACTION_CHANGE_ACCESS, $params );
	}

	/**
	 * Log when the user disables their own access
	 *
	 * @param UserIdentity $performer
	 */
	public function logAccessDisabled( UserIdentity $performer ): void {
		$params = [
			'4::changeType' => self::ACTION_ACCESS_DISABLED,
		];
		$this->log( $performer, $performer->getName(), self::ACTION_CHANGE_ACCESS, $params );
	}

	/**
	 * @param UserIdentity $performer
	 * @param string $tempUser
	 * @param string $action
	 * @param int $timestamp
	 * @param array|null $params
	 */
	private function debouncedLog(
		UserIdentity $performer,
		string $tempUser,
		string $action,
		int $timestamp,
		?array $params = []
	): void {
		$timestampMinusDelay = $timestamp - $this->delay;
		$actorId = $this->actorStore->findActorId( $performer, $this->dbw );
		if ( !$actorId ) {
			$this->log( $performer, $tempUser, $action, $params );
			return;
		}

		$logline = $this->dbw->newSelectQueryBuilder()
			->select( '*' )
			->from( 'logging' )
			->where( [
				'log_type' => self::LOG_TYPE,
				'log_action' => $action,
				'log_actor' => $actorId,
				'log_namespace' => NS_USER,
				'log_title' => $tempUser,
				'log_timestamp > ' . $this->dbw->addQuotes( $this->dbw->timestamp( $timestampMinusDelay ) )
			] )
			->fetchRow();

		if ( !$logline ) {
			$this->log( $performer, $tempUser, $action, $params, $timestamp );
		}
	}

	/**
	 * @param UserIdentity $performer
	 * @param string $tempUser
	 * @param string $action
	 * @param array $params
	 * @param int|null $timestamp
	 */
	private function log(
		UserIdentity $performer,
		string $tempUser,
		string $action,
		array $params,
		?int $timestamp = null
	): void {
		$logEntry = $this->createManualLogEntry( $action );
		$logEntry->setPerformer( $performer );
		$logEntry->setTarget( Title::makeTitle( NS_USER, $tempUser ) );
		$logEntry->setParameters( $params );

		if ( $timestamp ) {
			$logEntry->setTimestamp( wfTimestamp( TS_MW, $timestamp ) );
		}

		try {
			$logEntry->insert( $this->dbw );
		} catch ( DBError $e ) {
			$this->logger->critical(
				'CheckUser temporary account log entry was not recorded. ' .
				'This means checks can occur without being auditable. ' .
				'Immediate fix required.'
			);
		}
	}

	/**
	 * There is no `LogEntryFactory` (or `Logger::insert()` method) in MediaWiki Core to inject
	 * via the constructor so use this method to isolate the creation of `LogEntry` objects during
	 * testing.
	 *
	 * @private
	 *
	 * @param string $subtype
	 * @return ManualLogEntry
	 */
	protected function createManualLogEntry( string $subtype ): ManualLogEntry {
		return new ManualLogEntry( self::LOG_TYPE, $subtype );
	}
}
