<?php

namespace MediaWiki\CheckUser\Tests\Unit\CheckUser\Pagers;

use HashConfig;
use IndexPager;
use MediaWiki\CheckUser\CheckUser\Pagers\AbstractCheckUserPager;
use MediaWiki\CheckUser\CheckUserQueryInterface;
use MediaWiki\CheckUser\Tests\Integration\CheckUser\Pagers\DeAbstractedCheckUserPagerTest;
use MediaWiki\Html\FormOptions;
use MediaWiki\Tests\Unit\Libs\Rdbms\AddQuoterMock;
use MediaWikiUnitTestCase;
use Message;
use RequestContext;
use Wikimedia\Rdbms\FakeResultWrapper;
use Wikimedia\Rdbms\IReadableDatabase;
use Wikimedia\Rdbms\IResultWrapper;
use Wikimedia\Rdbms\Platform\SQLPlatform;
use Wikimedia\TestingAccessWrapper;
use Wikimedia\Timestamp\ConvertibleTimestamp;

/**
 * Test class for AbstractCheckUserPager class
 *
 * @group CheckUser
 *
 * @covers \MediaWiki\CheckUser\CheckUser\Pagers\AbstractCheckUserPager
 */
class AbstractCheckUserPagerTest extends MediaWikiUnitTestCase {
	public function testGetTimeRangeStringFirstAndLastEqual() {
		$object = $this->getMockBuilder( DeAbstractedCheckUserPagerTest::class )
			->disableOriginalConstructor()
			->onlyMethods( [ 'getFormattedTimestamp' ] )
			->getMock();
		$object->expects( $this->once() )
			->method( 'getFormattedTimestamp' )
			->willReturn( 'mock_formatted_timestamp' );
		$object = TestingAccessWrapper::newFromObject( $object );
		$this->assertSame(
			'mock_formatted_timestamp',
			$object->getTimeRangeString( '1653077137', '1653077137' ),
			'Return value of AbstractCheckUserPager::getTimeRangeString was not as expected.'
		);
	}

	public function testGetTimeRangeStringFirstAndLastNotEqual() {
		$object = $this->getMockBuilder( DeAbstractedCheckUserPagerTest::class )
			->disableOriginalConstructor()
			->onlyMethods( [ 'msg' ] )
			->getMock();
		// Mock the Message class to assert that the message is constructed correctly.
		$mockMessage = $this->createMock( Message::class );
		$mockMessage->expects( $this->once() )
			->method( 'dateTimeParams' )
			->with( '1653047635', '1653077137' )
			->willReturnSelf();
		$mockMessage->expects( $this->once() )
			->method( 'escaped' )
			->willReturn( 'mock_formatted_timestamp' );
		$object->expects( $this->once() )
			->method( 'msg' )
			->with( 'checkuser-time-range' )
			->willReturn( $mockMessage );
		$object = TestingAccessWrapper::newFromObject( $object );
		$this->assertSame(
			'mock_formatted_timestamp',
			$object->getTimeRangeString( '1653047635', '1653077137' ),
			'Return value of AbstractCheckUserPager::getTimeRangeString was not as expected.'
		);
	}

	/** @dataProvider provideGetIndexName */
	public function testGetIndexName( $table, $xfor, $expectedIndexValue ) {
		$objectUnderTest = $this->getMockBuilder( DeAbstractedCheckUserPagerTest::class )
			->disableOriginalConstructor()
			->getMock();
		$objectUnderTest = TestingAccessWrapper::newFromObject( $objectUnderTest );
		$objectUnderTest->xfor = $xfor;
		$this->assertSame(
			$expectedIndexValue,
			$objectUnderTest->getIndexName( $table ),
			'Index name is not as expected.'
		);
	}

	public static function provideGetIndexName() {
		return [
			'cu_changes with null xfor' => [
				CheckUserQueryInterface::CHANGES_TABLE, null, 'cuc_actor_ip_time'
			],
			'cu_private_event with null xfor' => [
				CheckUserQueryInterface::PRIVATE_LOG_EVENT_TABLE, null, 'cupe_actor_ip_time'
			],
			'cu_log_event with false xfor' => [
				CheckUserQueryInterface::LOG_EVENT_TABLE, false, 'cule_ip_hex_time'
			],
			'cu_private_event with true xfor' => [
				CheckUserQueryInterface::PRIVATE_LOG_EVENT_TABLE, true, 'cupe_xff_hex_time'
			],
		];
	}

	/** @dataProvider provideGetIpHexColumn */
	public function testGetIpHexColumn( $xfor, $table, $expectedReturnValue ) {
		$objectUnderTest = $this->getMockBuilder( DeAbstractedCheckUserPagerTest::class )
			->disableOriginalConstructor()
			->getMock();
		$objectUnderTest = TestingAccessWrapper::newFromObject( $objectUnderTest );
		$objectUnderTest->xfor = $xfor;
		$this->assertSame(
			$expectedReturnValue,
			$objectUnderTest->getIpHexColumn( $xfor, $table ),
			'Call to ::getIpHexColumn did not return the correct value.'
		);
	}

	public static function provideGetIpHexColumn() {
		return [
			'Table as cu_changes with xfor as false' => [
				false, CheckUserQueryInterface::CHANGES_TABLE, 'cuc_ip_hex'
			],
			'Table as cu_changes with xfor as true' => [
				true, CheckUserQueryInterface::CHANGES_TABLE, 'cuc_xff_hex'
			],
			'Table as cu_log_event with xfor as true' => [
				true, CheckUserQueryInterface::LOG_EVENT_TABLE, 'cule_xff_hex'
			],
			'Table as cu_private_event with xfor as false' => [
				false, CheckUserQueryInterface::PRIVATE_LOG_EVENT_TABLE, 'cupe_ip_hex'
			],
		];
	}

	/** @dataProvider provideGetTimestampField */
	public function testGetTimestampField( $table, $expectedTimestampField ) {
		$objectUnderTest = $this->getMockBuilder( DeAbstractedCheckUserPagerTest::class )
			->disableOriginalConstructor()
			->onlyMethods( [] )
			->getMock();
		$this->assertSame(
			$expectedTimestampField,
			$objectUnderTest->getTimestampField( $table ),
			'Return value of ::getTimestampField was not as expected.'
		);
	}

	public static function provideGetTimestampField() {
		return [
			'Table as null' => [ null, 'timestamp' ],
			'Table as cu_changes' => [
				CheckUserQueryInterface::CHANGES_TABLE, 'cuc_timestamp'
			],
			'Table as cu_log_event' => [
				CheckUserQueryInterface::LOG_EVENT_TABLE, 'cule_timestamp'
			],
			'Table as cu_private_event' => [
				CheckUserQueryInterface::PRIVATE_LOG_EVENT_TABLE, 'cupe_timestamp'
			],
		];
	}

	/** @dataProvider provideSetPeriodCondition */
	public function testSetPeriodCondition( $period, $fakeTime, $expected ) {
		ConvertibleTimestamp::setFakeTime( $fakeTime );
		$object = $this->getMockBuilder( DeAbstractedCheckUserPagerTest::class )
			->disableOriginalConstructor()
			->onlyMethods( [] )
			->getMock();
		# Mock the DB as a SQLPlatform as ::setPeriodCondition only uses methods
		# included in SQLPlatform.
		$object = TestingAccessWrapper::newFromObject( $object );
		$object->mDb = new SQLPlatform( new AddQuoterMock() );
		$object->opts = new FormOptions();
		$object->opts->add( 'period', '' );
		$object->opts->setValue( 'period', $period, true );
		# Call method under test
		$object->setPeriodCondition();
		if ( $expected ) {
			$expected = $object->mDb->timestamp( $expected );
			$this->assertArrayEquals(
				[ $expected, '' ],
				$object->getRangeOffsets(),
				false,
				false,
				'A different time period condition was generated than expected.'
			);
		} else {
			$this->assertArrayEquals(
				[ '', '' ],
				$object->getRangeOffsets(),
				false,
				false,
				'Time period conditions were generated when they were not supposed to be.'
			);
		}
	}

	public static function provideSetPeriodCondition() {
		return [
			'Empty period' => [ '', '1653047635', false ],
			'Period value for all' => [ 0, '1653047635', false ],
			'Period value for 7 days' => [ 7, '1653077137', '20220513000000' ],
			'Period value for 30 days' => [ 30, '1653047635', '20220420000000' ],
		];
	}

	public function testGetCheckUserHelperFieldsetWhenNoResults() {
		$object = $this->getMockBuilder( DeAbstractedCheckUserPagerTest::class )
			->disableOriginalConstructor()
			->getMock();
		$object = TestingAccessWrapper::newFromObject( $object );
		$object->mResult = $this->createMock( IResultWrapper::class );
		$object->mResult->method( 'numRows' )->willReturn( 0 );
		$this->assertNull(
			$object->getCheckUserHelperFieldset(),
			'The fieldset should not be shown if there are no results.'
		);
	}

	/** @dataProvider provideGetIpConds */
	public function testGetIpConds( $target, $table, $xfor, $expected ) {
		# Mock config on main request context.
		RequestContext::getMain()->setConfig(
			new HashConfig( [ 'CheckUserCIDRLimit' => [
				'IPv4' => 16,
				'IPv6' => 19,
			] ] )
		);
		$quoterMock = new AddQuoterMock();
		$this->assertEquals(
			$expected,
			AbstractCheckUserPager::getIpConds( $quoterMock, $target, $xfor, $table )
		);
	}

	public static function provideGetIpConds() {
		return [
			'Single IPv4 address for cu_changes and non-XFF' => [
				'212.35.31.121',
				CheckUserQueryInterface::CHANGES_TABLE,
				false,
				[ 'cuc_ip_hex' => 'D4231F79' ],
			],
			'Single IPv4 address for cu_private_event and XFF' => [
				'212.35.31.121',
				CheckUserQueryInterface::PRIVATE_LOG_EVENT_TABLE,
				true,
				[ 'cupe_xff_hex' => 'D4231F79' ],
			],
			'Single IPv4 address for cu_log_event and non-XFF' => [
				'212.35.31.121',
				CheckUserQueryInterface::LOG_EVENT_TABLE,
				false,
				[ 'cule_ip_hex' => 'D4231F79' ],
			],
			'Single IPv4 address notated as a /32' => [
				'212.35.31.121/32',
				CheckUserQueryInterface::CHANGES_TABLE,
				false,
				[ 0 => 'cuc_ip_hex BETWEEN \'D4231F79\' AND \'D4231F79\'' ],
			],
			'Single IPv6 address' => [
				'::e:f:2001',
				CheckUserQueryInterface::CHANGES_TABLE,
				false,
				[ 'cuc_ip_hex' => 'v6-00000000000000000000000E000F2001' ],
			],
			'IPv6 /96 range for cu_changes and non-XFF' => [
				'::e:f:2001/96',
				CheckUserQueryInterface::CHANGES_TABLE,
				false,
				[ 0 => 'cuc_ip_hex BETWEEN \'v6-00000000000000000000000E00000000\'' .
					' AND \'v6-00000000000000000000000EFFFFFFFF\'' ],
			],
			'IPv6 /96 range for cu_private_event and XFF' => [
				'::e:f:2001/96',
				CheckUserQueryInterface::PRIVATE_LOG_EVENT_TABLE,
				true,
				[ 0 => 'cupe_xff_hex BETWEEN \'v6-00000000000000000000000E00000000\'' .
					' AND \'v6-00000000000000000000000EFFFFFFFF\'' ],
			],
			'Invalid IP address' => [
				'abcedf',
				CheckUserQueryInterface::CHANGES_TABLE,
				false,
				false
			]
		];
	}

	public function testGetIpCondsLowerThanLimit() {
		$this->testGetIpConds(
			"0.17.184.5/15",
			CheckUserQueryInterface::CHANGES_TABLE,
			false,
			false
		);
		$this->testGetIpConds(
			"2000::/18",
			CheckUserQueryInterface::PRIVATE_LOG_EVENT_TABLE,
			true,
			false
		);
	}

	/** @dataProvider provideIsValidRange */
	public function testIsValidRange( $target, $expected ) {
		# Mock config on main request context.
		RequestContext::getMain()->setConfig(
			new HashConfig( [ 'CheckUserCIDRLimit' => [
				'IPv4' => 16,
				'IPv6' => 19,
			] ] )
		);
		$this->assertSame(
			$expected,
			AbstractCheckUserPager::isValidRange( $target )
		);
	}

	/**
	 * Test cases for AbstractCheckUserPager::isValid
	 */
	public static function provideIsValidRange() {
		return [
			'Single IPv4 address' => [ '212.35.31.121', true ],
			'Single IPv4 address notated as a /32' => [ '212.35.31.121/32', true ],
			'Single IPv6 address' => [ '::e:f:2001', true ],
			'IPv6 /96 range' => [ '::e:f:2001/96', true ],
			'Invalid IP address' => [ 'abcedf', false ]
		];
	}

	public function testIsValidRangeLowerThanLimit() {
		$this->testIsValidRange( "0.17.184.5/15", false );
		$this->testIsValidRange( "2000::/18", false );
	}

	/** @dataProvider provideBuildQueryInfo */
	public function testBuildQueryInfo(
		$offset, $limit, $order, $startOffset, $endOffset,
		$includeOffset, $eventTablesMigrationStage, $mockedQueryInfo,
		$partialExpectedArray
	) {
		$object = $this->getMockBuilder( DeAbstractedCheckUserPagerTest::class )
			->disableOriginalConstructor()
			->onlyMethods( [ 'getConfig', 'getQueryInfo' ] )
			->getMock();
		# Mock the config as this is a unit test.
		$object->method( 'getConfig' )
			->willReturn( new HashConfig( [ 'CheckUserEventTablesMigrationStage' => $eventTablesMigrationStage ] ) );
		if ( $eventTablesMigrationStage & SCHEMA_COMPAT_READ_NEW ) {
			# Mock ::getQueryInfo to return the query info for each three queries in turn.
			$object->method( 'getQueryInfo' )
				->withConsecutive(
					[ CheckUserQueryInterface::CHANGES_TABLE ],
					[ CheckUserQueryInterface::LOG_EVENT_TABLE ],
					[ CheckUserQueryInterface::PRIVATE_LOG_EVENT_TABLE ]
				)
				->willReturnOnConsecutiveCalls(
					$mockedQueryInfo[CheckUserQueryInterface::CHANGES_TABLE],
					$mockedQueryInfo[CheckUserQueryInterface::LOG_EVENT_TABLE],
					$mockedQueryInfo[CheckUserQueryInterface::PRIVATE_LOG_EVENT_TABLE]
				);
		} else {
			# Mock ::getQueryInfo to return the query info only for cu_changes.
			$object->method( 'getQueryInfo' )
				->with( CheckUserQueryInterface::CHANGES_TABLE )
				->willReturn( $mockedQueryInfo[CheckUserQueryInterface::CHANGES_TABLE] );
		}
		$object->setIncludeOffset( $includeOffset );
		$object->mDb = new SQLPlatform( new AddQuoterMock() );
		$object = TestingAccessWrapper::newFromObject( $object );
		# Set the start and end offset.
		$object->endOffset = $endOffset;
		$object->startOffset = $startOffset;
		# Needed because the constructor was disabled. These would be set by the constructor.
		$object->mIndexField = 'timestamp';
		$object->mExtraSortFields = [];
		# Call the method under test.
		$queryInfo = $object->buildQueryInfo( $offset, $limit, $order );
		if ( $eventTablesMigrationStage & SCHEMA_COMPAT_READ_NEW ) {
			$this->assertCount(
				3,
				$queryInfo,
				'::buildQueryInfo should have returned query info for three queries.'
			);
		} else {
			$this->assertCount(
				1,
				$queryInfo,
				'::buildQueryInfo should have returned query info for one query.'
			);
		}
		foreach ( $queryInfo as $table => $queryInfoForTable ) {
			$this->assertArrayContains(
				$partialExpectedArray[$table],
				$queryInfoForTable,
				"::buildQueryInfo result was not correct for table $table."
			);
		}
	}

	public static function provideBuildQueryInfo() {
		return [
			'No offset, limit 500, order DESC while reading old' => [
				'', 500, IndexPager::QUERY_DESCENDING, '', '',
				false, SCHEMA_COMPAT_READ_OLD,
				[ CheckUserQueryInterface::CHANGES_TABLE => [
					'tables' => [ 'cu_changes' ],
					'fields' => [ 'timestamp', 'alias' => 'test' ],
					'conds' => [ 'cond' => 'test' ],
					'options' => [],
					'join_conds' => []
				] ],
				[ CheckUserQueryInterface::CHANGES_TABLE => [
					# tables
					0 => [ 'cu_changes' ],
					# fields
					1 => [ 'timestamp', 'alias' => 'test' ],
					# where conds
					2 => [ 'cond' => 'test' ],
					# options
					4 => [ 'LIMIT' => 500, 'ORDER BY' => [ 'timestamp DESC' ] ],
					# join_conds
					5 => [],
				] ]
			],
			'Start and end offset, limit 3, order DESC while reading old' => [
				'', 3, IndexPager::QUERY_DESCENDING, 'test_start_offset', 'test_end_offset',
				false, SCHEMA_COMPAT_READ_OLD,
				[ CheckUserQueryInterface::CHANGES_TABLE => [
					'tables' => [ 'cu_changes' ],
					'fields' => [ 'timestamp', 'alias' => 'test' ],
					'conds' => [ 'cond' => 'test' ],
					'options' => [],
					'join_conds' => []
				] ],
				[ CheckUserQueryInterface::CHANGES_TABLE => [
					# tables
					0 => [ 'cu_changes' ],
					# fields
					1 => [ 'timestamp', 'alias' => 'test' ],
					# where conds
					2 => [
						'cond' => 'test',
						"cuc_timestamp < 'test_end_offset'",
						"cuc_timestamp >= 'test_start_offset'"
					],
					# options
					4 => [ 'LIMIT' => 3, 'ORDER BY' => [ 'timestamp DESC' ] ],
					# join_conds
					5 => [],
				] ]
			],
			'Offset, limit 20, order ASC, include offset while reading new' => [
				'test_offset', 20, IndexPager::QUERY_ASCENDING, '', '',
				true, SCHEMA_COMPAT_READ_NEW,
				[
					CheckUserQueryInterface::CHANGES_TABLE => [
						'tables' => [ 'cu_changes' ],
						'fields' => [ 'timestamp', 'alias' => 'test' ],
						'conds' => [ 'cond' => 'test' ],
						'options' => [],
						'join_conds' => []
					],
					CheckUserQueryInterface::LOG_EVENT_TABLE => [
						'tables' => [ 'cu_log_event' ],
						'fields' => [ 'timestamp', 'alias' => 'test2' ],
						'conds' => [ 'cond' => 'test2' ],
						'options' => [],
						'join_conds' => [ 'logging' => 'test' ]
					],
					CheckUserQueryInterface::PRIVATE_LOG_EVENT_TABLE => [
						'tables' => [ 'cu_private_event' ],
						'fields' => [ 'timestamp', 'alias' => 'test3' ],
						'conds' => [ 'cond' => 'test3' ],
						'options' => [],
						'join_conds' => []
					],
				],
				[
					CheckUserQueryInterface::CHANGES_TABLE => [
						# tables
						0 => [ 'cu_changes' ],
						# fields
						1 => [ 'timestamp', 'alias' => 'test' ],
						# where conds
						2 => [ 'cond' => 'test', "cuc_timestamp >= 'test_offset'" ],
						# options
						4 => [ 'LIMIT' => 20, 'ORDER BY' => [ 'timestamp' ] ],
						# join_conds
						5 => [],
					],
					CheckUserQueryInterface::LOG_EVENT_TABLE => [
						# tables
						0 => [ 'cu_log_event' ],
						# fields
						1 => [ 'timestamp', 'alias' => 'test2' ],
						# where conds
						2 => [ 'cond' => 'test2', "cule_timestamp >= 'test_offset'" ],
						# options
						4 => [ 'LIMIT' => 20, 'ORDER BY' => [ 'timestamp' ] ],
						# join_conds
						5 => [ 'logging' => 'test' ],
					],
					CheckUserQueryInterface::PRIVATE_LOG_EVENT_TABLE => [
						# tables
						0 => [ 'cu_private_event' ],
						# fields
						1 => [ 'timestamp', 'alias' => 'test3' ],
						# where conds
						2 => [ 'cond' => 'test3', "cupe_timestamp >= 'test_offset'" ],
						# options
						4 => [ 'LIMIT' => 20, 'ORDER BY' => [ 'timestamp' ] ],
						# join_conds
						5 => [],
					],
				]
			],
		];
	}

	/** @dataProvider provideReallyDoQuery */
	public function testReallyDoQuery(
		$limit, $order, $eventTableMigrationStage, $fakeResults, $expectedReturnResults
	) {
		$object = $this->getMockBuilder( DeAbstractedCheckUserPagerTest::class )
			->disableOriginalConstructor()
			->onlyMethods( [ 'buildQueryInfo' ] )
			->getMock();
		// Mock the DB
		$mockDb = $this->createMock( IReadableDatabase::class );
		// Mock the select method
		$mockedQueryInfoForCuChanges = [ [ 'cu_changes' ], [ 'timestamp' ], [], 'fname', [], [] ];
		$mockedQueryInfoForCuLogEvent = [ [ 'cu_log_event' ], [ 'timestamp' ], [], 'fname', [], [] ];
		$mockedQueryInfoForCuPrivateEvent = [ [ 'cu_private_event' ], [ 'timestamp' ], [], 'fname', [], [] ];
		if ( $eventTableMigrationStage & SCHEMA_COMPAT_READ_NEW ) {
			$object->expects( $this->once() )
				->method( 'buildQueryInfo' )
				->willReturn( [
					$mockedQueryInfoForCuChanges, $mockedQueryInfoForCuLogEvent, $mockedQueryInfoForCuPrivateEvent
				] );
			$mockDb->expects( $this->exactly( 3 ) )
				->method( 'select' )
				->withConsecutive(
					$mockedQueryInfoForCuChanges, $mockedQueryInfoForCuLogEvent, $mockedQueryInfoForCuPrivateEvent
				)
				->willReturnOnConsecutiveCalls(
					$fakeResults[CheckUserQueryInterface::CHANGES_TABLE],
					$fakeResults[CheckUserQueryInterface::LOG_EVENT_TABLE],
					$fakeResults[CheckUserQueryInterface::PRIVATE_LOG_EVENT_TABLE]
				);
		} else {
			$object->expects( $this->once() )
				->method( 'buildQueryInfo' )
				->willReturn( [ $mockedQueryInfoForCuChanges ] );
			$mockDb->expects( $this->once() )
				->method( 'select' )
				->withConsecutive( $mockedQueryInfoForCuChanges )
				->willReturn( $fakeResults[CheckUserQueryInterface::CHANGES_TABLE] );
		}
		$object->mDb = $mockDb;
		$returnResults = $object->reallyDoQuery( '', $limit, $order );
		$this->assertSame(
			count( $expectedReturnResults ),
			$returnResults->count(),
			'Unexpected number of results returned by ::reallyDoQuery.'
		);
		if ( $returnResults->count() ) {
			$fetchRowResult = $returnResults->fetchRow();
			while ( $fetchRowResult ) {
				$this->assertArrayEquals(
					$expectedReturnResults[$returnResults->key()],
					$fetchRowResult,
					false,
					false,
					"Returned results from ::reallyDoQuery at index {$returnResults->key()} were not as expected."
				);
				$fetchRowResult = $returnResults->fetchRow();
			}
		}
	}

	public static function provideReallyDoQuery() {
		# Generate rows to return from IDatabase::select with just the timestamp for testing.
		$fakeResultsOrderDesc = [];
		$fakeResultsOrderAsc = [];
		$fakeResultsPerTable = [];
		$fakeResultsOrderDescForReadOld = [];
		$fakeResultsOrderAscForReadOld = [];
		for ( $i = 0; $i < 10; $i++ ) {
			foreach ( CheckUserQueryInterface::RESULT_TABLES as $tableIndex => $table ) {
				$row = [ 'timestamp' => '202301052123' . $i . $tableIndex, 'title' => 'Test' . $table . $i ];
				$fakeResultsOrderDesc['202301052123' . $i . $tableIndex] = $row;
				$fakeResultsOrderAsc['202301052123' . $i . $tableIndex] = $row;
				if ( $table === CheckUserQueryInterface::CHANGES_TABLE ) {
					$fakeResultsOrderDescForReadOld['202301052123' . $i . $tableIndex] = $row;
					$fakeResultsOrderAscForReadOld['202301052123' . $i . $tableIndex] = $row;
				}
				$fakeResultsPerTable[$table][] = $row;
			}
		}
		# Wrap the results in a FakeResultWrapper
		$fakeResultsPerTable = array_map( static function ( $results_array ) {
			return new FakeResultWrapper( $results_array );
		}, $fakeResultsPerTable );
		# Generate the expected return results for different conditions.
		krsort( $fakeResultsOrderDescForReadOld );
		$fakeResultsOrderDescForReadOld = array_values( $fakeResultsOrderDescForReadOld );
		ksort( $fakeResultsOrderAscForReadOld );
		$fakeResultsOrderAscForReadOld = array_values( $fakeResultsOrderAscForReadOld );
		krsort( $fakeResultsOrderDesc );
		$fakeResultsOrderDesc = array_values( $fakeResultsOrderDesc );
		ksort( $fakeResultsOrderAsc );
		$fakeResultsOrderAsc = array_values( $fakeResultsOrderAsc );
		# Start test cases
		return [
			'Limit 500, order DESC while reading old' => [
				500, IndexPager::QUERY_DESCENDING, SCHEMA_COMPAT_READ_OLD,
				$fakeResultsPerTable, $fakeResultsOrderDescForReadOld
			],
			'Limit 500, order ASC while reading new' => [
				500, IndexPager::QUERY_ASCENDING, SCHEMA_COMPAT_READ_NEW,
				$fakeResultsPerTable, $fakeResultsOrderAsc
			],
			'Limit 10, order DESC while reading old' => [
				10, IndexPager::QUERY_DESCENDING, SCHEMA_COMPAT_READ_OLD,
				$fakeResultsPerTable, array_slice( $fakeResultsOrderDescForReadOld, 0, 10 )
			],
			'Limit 10, order DESC while reading new' => [
				10, IndexPager::QUERY_DESCENDING, SCHEMA_COMPAT_READ_NEW,
				$fakeResultsPerTable, array_slice( $fakeResultsOrderDesc, 0, 10 )
			],
			'Limit 5, order ASC while reading old' => [
				5, IndexPager::QUERY_ASCENDING, SCHEMA_COMPAT_READ_OLD,
				$fakeResultsPerTable, array_slice( $fakeResultsOrderAscForReadOld, 0, 5 )
			],
			'Limit 10, order ASC while reading new' => [
				10, IndexPager::QUERY_ASCENDING, SCHEMA_COMPAT_READ_NEW,
				$fakeResultsPerTable, array_slice( $fakeResultsOrderAsc, 0, 10 )
			],
		];
	}
}
