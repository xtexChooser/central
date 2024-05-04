<?php

namespace MediaWiki\Extension\OAuth\Tests\Backend;

use MediaWiki\Extension\OAuth\Backend\Hooks;
use MediaWiki\MediaWikiServices;
use MediaWiki\Status\Status;
use MediaWiki\User\User;
use PHPUnit\Framework\TestCase;

/**
 * @covers \MediaWiki\Extension\OAuth\Backend\MWOAuthServer
 * @group OAuth
 */
class MWOAuthHooksTest extends TestCase {

	/**
	 * @dataProvider provideOnChangeTagCanCreate
	 */
	public function testOnChangeTagCanCreate( $tagName, $statusOk ) {
		$status = Status::newGood();
		$services = MediaWikiServices::getInstance();

		$hooks = new Hooks( $services->getChangeTagDefStore(), $services->getConnectionProvider() );
		$hooks->onChangeTagCanCreate( $tagName, new User, $status );
		$this->assertSame( $statusOk, $status->isOK() );
	}

	public static function provideOnChangeTagCanCreate() {
		return [
			[ 'foo', true ],
			[ 'OAuth CID', true ],
			[ 'OAuth CID:', false ],
			[ 'oauth cid:', false ],
			[ 'OAuth CID: 123', false ],
		];
	}

}
