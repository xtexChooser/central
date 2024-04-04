<?php

namespace MediaWiki\CheckUser\Tests\Unit\CheckUser\Pagers;

use HashConfig;
use Language;
use MediaWiki\CheckUser\CheckUser\Pagers\CheckUserGetEditsPager;
use MediaWiki\CheckUser\Services\UserAgentClientHintsManager;
use MediaWiki\CommentFormatter\CommentFormatter;
use MediaWiki\CommentStore\CommentStore;
use MediaWiki\User\UserIdentityValue;
use RequestContext;
use Wikimedia\IPUtils;
use Wikimedia\Rdbms\IReadableDatabase;
use Wikimedia\TestingAccessWrapper;

/**
 * Test class for CheckUserGetEditsPager class
 *
 * @group CheckUser
 *
 * @covers \MediaWiki\CheckUser\CheckUser\Pagers\CheckUserGetEditsPager
 */
class CheckUserGetEditsPagerTest extends CheckUserPagerCommonUnitTest {

	/** @inheritDoc */
	protected function getPagerClass(): string {
		return CheckUserGetEditsPager::class;
	}

	/** @dataProvider provideIsNavigationBarShown */
	public function testIsNavigationBarShown( $numRows, $shown ) {
		$object = $this->getMockBuilder( CheckUserGetEditsPager::class )
			->onlyMethods( [ 'getNumRows' ] )
			->disableOriginalConstructor()
			->getMock();
		$object->expects( $this->once() )
			->method( 'getNumRows' )
			->willReturn( $numRows );
		$object = TestingAccessWrapper::newFromObject( $object );
		if ( $shown ) {
			$this->assertTrue(
				$object->isNavigationBarShown(),
				'Navigation bar is not showing when it\'s supposed to'
			);
		} else {
			$this->assertFalse(
				$object->isNavigationBarShown(),
				'Navigation bar is showing when it is not supposed to'
			);
		}
	}

	public static function provideIsNavigationBarShown() {
		return [
			[ 0, false ],
			[ 2, true ]
		];
	}

	/** @dataProvider provideGetQueryInfo */
	public function testGetQueryInfo( $table, $tableSpecificQueryInfo, $expectedQueryInfo ) {
		# Mock config on main request context for ::getIpConds which is static
		# and gets the config from the main request.
		RequestContext::getMain()->setConfig(
			new HashConfig( [ 'CheckUserCIDRLimit' => [
				'IPv4' => 16,
				'IPv6' => 19,
			] ] )
		);
		$this->commonTestGetQueryInfo(
			UserIdentityValue::newAnonymous( '127.0.0.1' ), false,
			$table, $tableSpecificQueryInfo, $expectedQueryInfo
		);
	}

	public static function provideGetQueryInfo() {
		return [
			'cu_changes table' => [
				'cu_changes', [
					'tables' => [ 'cu_changes' ],
					'conds' => [ 'cuc_only_for_read_old' => 0 ],
					'fields' => [], 'options' => [], 'join_conds' => [],
				],
				[
					'tables' => [ 'cu_changes' ],
					'conds' => [ 'cuc_ip_hex' => IPUtils::toHex( '127.0.0.1' ), 'cuc_only_for_read_old' => 0 ],
					'fields' => [],
					'options' => [ 'USE INDEX' => [ 'cu_changes' => 'cuc_ip_hex_time' ] ],
					'join_conds' => [],
				]
			],
			'cu_log_event table' => [
				'cu_log_event', [
					'tables' => [ 'cu_log_event' ], 'conds' => [],
					'fields' => [], 'options' => [], 'join_conds' => [],
				],
				[
					'tables' => [ 'cu_log_event' ],
					'conds' => [ 'cule_ip_hex' => IPUtils::toHex( '127.0.0.1' ) ],
					'fields' => [],
					'options' => [ 'USE INDEX' => [ 'cu_log_event' => 'cule_ip_hex_time' ] ],
					'join_conds' => [],
				]
			],
			'cu_private_event table' => [
				'cu_private_event', [
					'tables' => [ 'cu_private_event' ], 'conds' => [],
					'fields' => [], 'options' => [], 'join_conds' => [],
				],
				[
					'tables' => [ 'cu_private_event' ],
					'conds' => [ 'cupe_ip_hex' => IPUtils::toHex( '127.0.0.1' ) ],
					'fields' => [],
					'options' => [ 'USE INDEX' => [ 'cu_private_event' => 'cupe_ip_hex_time' ] ],
					'join_conds' => [],
				]
			],
		];
	}

	/** @dataProvider provideGetQueryInfoForCuChanges */
	public function testGetQueryInfoForCuChanges( $eventTableMigrationStage, $displayClientHints, $expectedQueryInfo ) {
		$this->commonGetQueryInfoForTableSpecificMethod(
			'getQueryInfoForCuChanges',
			[
				'eventTableReadNew' => boolval( $eventTableMigrationStage & SCHEMA_COMPAT_READ_NEW ),
				'commentStore' => new CommentStore( $this->createMock( Language::class ) ),
				'displayClientHints' => $displayClientHints
			],
			$expectedQueryInfo
		);
	}

	public static function provideGetQueryInfoForCuChanges() {
		return [
			'Returns expected keys to arrays and includes cu_changes in tables while reading new' => [
				SCHEMA_COMPAT_READ_NEW,
				false,
				[
					# Fields should be an array
					'fields' => [],
					# Assert at least cu_changes in the table list
					'tables' => [ 'cu_changes' ],
					# When reading new, do not include rows from cu_changes
					# that were marked as only being for read old.
					'conds' => [ 'cuc_only_for_read_old' => 0 ],
					# Should be all of these as arrays
					'options' => [],
					'join_conds' => [],
				]
			],
			'Returns expected keys to arrays and includes cu_changes in tables while reading old' => [
				SCHEMA_COMPAT_READ_OLD,
				false,
				[
					# Fields should be an array
					'fields' => [],
					# Assert at least cu_changes in the table list
					'tables' => [ 'cu_changes' ],
					# Should be all of these as arrays
					'conds' => [],
					'options' => [],
					'join_conds' => [],
				]
			],
			'When displaying client hints' => [
				SCHEMA_COMPAT_NEW,
				true,
				[
					# Fields should be an array with Client Hints fields set.
					'fields' => [
						'client_hints_reference_id' => 'cuc_this_oldid',
						'client_hints_reference_type' => UserAgentClientHintsManager::IDENTIFIER_CU_CHANGES,
					],
					# Tables array should have at least cu_log_event
					'tables' => [ 'cu_changes' ],
					'conds' => [ 'cuc_only_for_read_old' => 0 ],
					'options' => [],
					'join_conds' => [],
				]
			],
		];
	}

	/** @dataProvider provideGetQueryInfoForCuLogEvent */
	public function testGetQueryInfoForCuLogEvent( $displayClientHints, $databaseType, $expectedQueryInfo ) {
		$mockDbr = $this->createMock( IReadableDatabase::class );
		$mockDbr->expects( $this->once() )
			->method( 'getType' )
			->willReturn( $databaseType );
		$this->commonGetQueryInfoForTableSpecificMethod(
			'getQueryInfoForCuLogEvent',
			[
				'commentStore' => new CommentStore( $this->createMock( Language::class ) ),
				'displayClientHints' => $displayClientHints,
				'mDb' => $mockDbr,
			],
			$expectedQueryInfo
		);
	}

	public static function provideGetQueryInfoForCuLogEvent() {
		return [
			'Returns expected keys to arrays and includes cu_log_event in tables' => [
				false,
				'mysql',
				[
					# Fields should be an array
					'fields' => [],
					# Tables array should have at least cu_log_event
					'tables' => [ 'cu_log_event' ],
					# All other values should be arrays
					'conds' => [],
					'options' => [],
					'join_conds' => [],
				]
			],
			'When displaying client hints' => [
				true,
				'mysql',
				[
					# Fields should be an array with Client Hints fields set.
					'fields' => [
						'client_hints_reference_id' => 'cule_log_id',
						'client_hints_reference_type' => UserAgentClientHintsManager::IDENTIFIER_CU_LOG_EVENT,
					],
					# Tables array should have at least cu_log_event
					'tables' => [ 'cu_log_event' ],
					# All other values should be arrays
					'conds' => [],
					'options' => [],
					'join_conds' => [],
				]
			],
			'When using postgres DB' => [
				false,
				'postgres',
				[
					# Fields should be an array with type casted to a smallint when using postgres DB.
					'fields' => [ 'type' => 'CAST(3 AS smallint)' ],
					# Tables array should have at least cu_log_event
					'tables' => [ 'cu_log_event' ],
					# All other values should be arrays
					'conds' => [],
					'options' => [],
					'join_conds' => [],
				]
			],
		];
	}

	/** @dataProvider provideGetQueryInfoForCuPrivateEvent */
	public function testGetQueryInfoForCuPrivateEvent( $displayClientHints, $databaseType, $expectedQueryInfo ) {
		$mockDbr = $this->createMock( IReadableDatabase::class );
		$mockDbr->expects( $this->once() )
			->method( 'getType' )
			->willReturn( $databaseType );
		$this->commonGetQueryInfoForTableSpecificMethod(
			'getQueryInfoForCuPrivateEvent',
			[
				'commentStore' => new CommentStore( $this->createMock( Language::class ) ),
				'displayClientHints' => $displayClientHints,
				'mDb' => $mockDbr,
			],
			$expectedQueryInfo
		);
	}

	public static function provideGetQueryInfoForCuPrivateEvent() {
		return [
			'Returns expected keys to arrays and includes cu_log_event in tables' => [
				false,
				'mysql',
				[
					# Fields should be an array
					'fields' => [ 'type' => RC_LOG ],
					# Tables array should have at least cu_private_event
					'tables' => [ 'cu_private_event' ],
					# All other values should be arrays
					'conds' => [],
					'options' => [],
					'join_conds' => [],
				]
			],
			'When displaying client hints' => [
				true,
				'mysql',
				[
					# Fields should be an array with Client Hints fields set.
					'fields' => [
						'client_hints_reference_id' => 'cupe_id',
						'client_hints_reference_type' => UserAgentClientHintsManager::IDENTIFIER_CU_PRIVATE_EVENT,
					],
					# Tables array should have at least cu_private_event
					'tables' => [ 'cu_private_event' ],
					# All other values should be arrays
					'conds' => [],
					'options' => [],
					'join_conds' => [],
				]
			],
			'When using postgres DB' => [
				false,
				'postgres',
				[
					# Fields should be an array with type casted to a smallint when using postgres DB.
					'fields' => [ 'type' => 'CAST(3 AS smallint)' ],
					# Tables array should have at least cu_private_event
					'tables' => [ 'cu_private_event' ],
					# All other values should be arrays
					'conds' => [],
					'options' => [],
					'join_conds' => [],
				]
			],
		];
	}

	/** @dataProvider provideGetActionTextForReadOld */
	public function testGetActionTextForReadOld( $row, $expectedActionText ) {
		$commentFormatterMock = $this->createMock( CommentFormatter::class );
		$commentFormatterMock->method( 'format' )
			->willReturnArgument( 0 );
		$objectUnderTest = $this->getMockBuilder( CheckUserGetEditsPager::class )
			->onlyMethods( [] )
			->disableOriginalConstructor()
			->getMock();
		$objectUnderTest = TestingAccessWrapper::newFromObject( $objectUnderTest );
		$objectUnderTest->commentFormatter = $commentFormatterMock;
		$objectUnderTest->eventTableReadNew = true;
		$this->assertSame(
			$expectedActionText,
			$objectUnderTest->getActionText( (object)$row, UserIdentityValue::newAnonymous( '127.0.0.1' ) ),
			'::getActionText did not return the correct actiontext.'
		);
	}

	public static function provideGetActionTextForReadOld() {
		return [
			'Action text as null' => [
				[
					'actiontext' => null,
					'type' => RC_LOG,
				],
				''
			],
			'Action text as empty string' => [
				[
					'actiontext' => '',
					'type' => RC_LOG,
					'log_type' => null,
				],
				''
			],
			'Action text as a non-empty string' => [
				[
					'actiontext' => 'testing',
					'type' => RC_LOG,
				],
				'testing'
			],
		];
	}
}
