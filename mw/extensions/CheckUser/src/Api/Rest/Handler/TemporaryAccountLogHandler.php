<?php

namespace MediaWiki\CheckUser\Api\Rest\Handler;

use MediaWiki\Rest\LocalizedHttpException;
use Wikimedia\Message\DataMessageValue;
use Wikimedia\Message\MessageValue;
use Wikimedia\ParamValidator\ParamValidator;
use Wikimedia\Rdbms\IDatabase;

class TemporaryAccountLogHandler extends AbstractTemporaryAccountHandler {
	/**
	 * @inheritDoc
	 */
	protected function getData( int $actorId, IDatabase $dbr ): array {
		if (
			!( $this->config->get( 'CheckUserEventTablesMigrationStage' ) &
				SCHEMA_COMPAT_READ_NEW )
		) {
			// Pretend the route doesn't exist
			throw new LocalizedHttpException(
				new MessageValue( 'rest-no-match' ), 404
			);
		}

		$ids = $this->getValidatedParams()['ids'];
		if ( !count( $ids ) ) {
			throw new LocalizedHttpException(
				DataMessageValue::new( 'paramvalidator-missingparam', [], 'missingparam' )
					->plaintextParams( "ids" ),
				400,
				[
					'error' => 'parameter-validation-failed',
					'name' => 'ids',
					'value' => '',
					'failureCode' => "missingparam",
					'failureData' => null,
				]
			);
		}
		$conds = [
			'cule_actor' => $actorId,
			'cule_log_id' => $ids,
		];

		$rows = $this->loadBalancer->getConnection( DB_REPLICA )
			->newSelectQueryBuilder()
			// T327906: 'cule_actor' and 'cule_timestamp' are selected
			// only to satisfy Postgres requirement where all ORDER BY
			// fields must be present in SELECT list.
			->select( [ 'cule_log_id', 'cule_ip','cule_actor', 'cule_timestamp' ] )
			->from( 'cu_log_event' )
			->where( $conds )
			->orderBy( [ 'cule_actor', 'cule_ip', 'cule_timestamp' ] )
			->caller( __METHOD__ )
			->fetchResultSet();

		$ips = [];
		foreach ( $rows as $row ) {
			// In the unlikely case that there are rows with the same
			// log ID, the final array will contain the most recent
			$ips[$row->cule_log_id] = $row->cule_ip;
		}

		return [ 'ips' => $ips ];
	}

	/**
	 * @inheritDoc
	 */
	public function getParamSettings() {
		$settings = parent::getParamSettings();
		$settings['ids'] = [
			self::PARAM_SOURCE => 'path',
			ParamValidator::PARAM_TYPE => 'integer',
			ParamValidator::PARAM_REQUIRED => true,
			ParamValidator::PARAM_ISMULTI => true,
		];
		return $settings;
	}
}
