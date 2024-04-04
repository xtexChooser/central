<?php

namespace MediaWiki\CheckUser\Tests\Integration\CheckUser\Pagers;

use MediaWiki\CheckUser\CheckUser\SpecialCheckUser;
use MediaWiki\CheckUser\Hooks;
use MediaWiki\CheckUser\Tests\TemplateParserMockTest;
use MediaWiki\User\UserIdentityValue;
use Wikimedia\IPUtils;
use Wikimedia\TestingAccessWrapper;

/**
 * Test class for CheckUserGetIPsPager class
 *
 * @group CheckUser
 * @group Database
 *
 * @covers \MediaWiki\CheckUser\CheckUser\Pagers\CheckUserGetIPsPager
 */
class CheckUserGetIPsPagerTest extends CheckUserPagerCommonTest {

	protected function setUp(): void {
		parent::setUp();

		$this->tablesUsed = array_merge(
			$this->tablesUsed,
			[
				'cu_changes',
				'cu_log',
			]
		);

		$this->checkSubtype = SpecialCheckUser::SUBTYPE_GET_IPS;
		$this->defaultUserIdentity = $this->getTestUser()->getUserIdentity();
		$this->defaultCheckType = 'userips';
	}

	/** @dataProvider provideGetCountForIPEdits */
	public function testGetCountForIPEdits( $ips, $performers, $expectedCount ) {
		$object = $this->setUpObject();
		$testUser = $this->getTestUser();
		$object->target = $testUser->getUserIdentity();
		$hooks = TestingAccessWrapper::newFromClass( Hooks::class );
		foreach ( $ips as $i => $ip ) {
			$row = [ 'cuc_ip' => $ip, 'cuc_ip_hex' => IPUtils::toHex( $ip ) ];
			$performer = $performers[$i];
			if ( IPUtils::isIPAddress( $performer ) ) {
				$performer = UserIdentityValue::newAnonymous( $performer );
			} else {
				$performer = $testUser->getUserIdentity();
			}
			$hooks->insertIntoCuChangesTable( $row, __METHOD__, $performer );
		}
		$this->assertSame(
			$expectedCount,
			$object->getCountForIPedits( '127.0.0.1' ),
			'The expected count for edits made by the IP was not returned.'
		);
	}

	public static function provideGetCountForIPEdits() {
		return [
			'One IP with only 1 user edit' => [
				[
					'127.0.0.1',
					'127.0.0.1',
					'127.0.0.1'
				],
				[
					'127.0.0.1',
					'Test user',
					'127.0.0.1'
				],
				3
			],
			'Two IPs with only 1 user edit' => [
				[
					'127.0.0.1',
					'127.0.0.1',
					'127.0.0.2',
					'127.0.0.2'
				],
				[
					'127.0.0.1',
					'Test user',
					'127.0.0.2',
					'Test user'
				],
				2
			]
		];
	}

	/**
	 * Tests that the template parameters provided to the GetIPsLine.mustache match
	 * the expected values. Does not test the mustache file which includes some
	 * conditional logic, HTML and whitespace.
	 *
	 * @dataProvider provideFormatRow
	 */
	public function testFormatRow( $row, $expectedTemplateParams ) {
		$object = $this->setUpObject();
		$object->templateParser = new TemplateParserMockTest();
		$row = array_merge( $this->getDefaultRowFieldValues(), $row );
		$object->formatRow( (object)$row );
		$this->assertNotNull(
			$object->templateParser->lastCalledWith,
			'The template parser was not called by formatRow.'
		);
		$this->assertSame(
			'GetIPsLine',
			$object->templateParser->lastCalledWith[0],
			'formatRow did not call the correct mustache file.'
		);
		$this->assertArrayEquals(
			$expectedTemplateParams,
			array_filter(
				$object->templateParser->lastCalledWith[1],
				static function ( $key ) use ( $expectedTemplateParams ) {
					return array_key_exists( $key, $expectedTemplateParams );
				},
				ARRAY_FILTER_USE_KEY
			),
			false,
			true,
			'The template parameters do not match the expected template parameters. If changes have been ' .
			'made to the template parameters make sure you update the tests.'
		);
	}

	public static function provideFormatRow() {
		// @todo test the rest of the template parameters.
		return [
			'Test edit count' => [
				[ 'count' => 555 ],
				[ 'editCount' => 555 ]
			],
		];
	}

	/** @inheritDoc */
	public function getDefaultRowFieldValues(): array {
		return [
			'ip' => '127.0.0.1',
			'ip_hex' => IPUtils::toHex( '127.0.0.1' ),
			'count' => 1,
			'first' => $this->db->timestamp(),
			'last' => $this->db->timestamp(),
		];
	}
}
