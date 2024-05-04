<?php

namespace MediaWiki\Extension\OAuth\Tests\Rest;

use Exception;
use FormatJson;
use MediaWiki\Extension\OAuth\Rest\Handler\RequestClient;
use MediaWiki\Rest\Handler;
use MediaWiki\Rest\ResponseInterface;
use MediaWiki\User\User;
use MediaWiki\WikiMap\WikiMap;
use MediaWikiIntegrationTestCase;

/**
 * @covers \MediaWiki\Extension\OAuth\Rest\Handler\RequestClient
 * @group Database
 * @group OAuth
 */
class RequestClientEndpointTest extends EndpointTestBase {

	/**
	 * @var array
	 */
	private const DEFAULT_POST_PARAMS = [
		'name' => 'TestName',
		'version' => '1.0',
		'description' => 'TestDescription',
		'wiki' => '*',
		'owner_only' => false,
		'callback_url' => 'https://test.com/oauth',
		'callback_is_prefix' => false,
		'email' => 'test@test.com',
		'is_confidential' => false,
		'grant_types' => [ 'client_credentials' ],
		'scopes' => [],
	];

	/**
	 * @var array
	 */
	private const POST_PARAMS_OWNERS_ONLY_RESTRICTION = [
		'callback_url' => false,
	];

	/**
	 * @var array
	 */
	private const POST_PARAMS_EMAIL_MISTMATCH = [
		'email' => '_test@test.com',
	];

	/**
	 * @var array
	 */
	private const POST_PARAMS_WRONG_GRANT_TYPES = [
		'owner_only' => true,
		'grant_types' => [ 'authorization_code', 'refresh_token' ],
	];

	/**
	 * @var array
	 */
	private const POST_PARAMS_OWNERS_ONLY = [
		'owner_only' => true,
	];

	/**
	 * @throws Exception
	 */
	protected function setUp(): void {
		parent::setUp();

		$this->setMwGlobals( [
			'wgMWOAuthCentralWiki' => WikiMap::getCurrentWikiId(),
			'wgGroupPermissions' => [
				'*' => [ 'mwoauthproposeconsumer' => true ]
			],
			'wgEmailAuthentication' => false
		] );
	}

	public static function provideTestHandlerExecute() {
		return [
			'No POST params' => [
				[
					'method' => 'POST',
					'uri' => self::makeUri( '/oauth2/client' ),
					'postParams' => []
				],
				[
					'statusCode' => 400,
					'reasonPhrase' => 'Bad Request',
					'protocolVersion' => '1.1'
				]
			],
			'Not logged in' => [
				[
					'method' => 'POST',
					'uri' => self::makeUri( '/oauth2/client' ),
					'postParams' => self::DEFAULT_POST_PARAMS,
					'headers' => [
						'Content-Type' => 'application/json'
					],
				],
				[
					'statusCode' => 400,
					'reasonPhrase' => 'Bad Request',
					'protocolVersion' => '1.1'
				],
			],
			'Email not confirmed' => [
				[
					'method' => 'POST',
					'uri' => self::makeUri( '/oauth2/client' ),
					'postParams' => self::DEFAULT_POST_PARAMS,
					'headers' => [
						'Content-Type' => 'application/json'
					],
				],
				[
					'statusCode' => 400,
					'reasonPhrase' => 'Bad Request',
					'protocolVersion' => '1.1'
				],
				static function () {
					return User::createNew( 'RequestClientTestUser1' );
				}
			],
			'Missing Callback URL for non-OwnerOnly client' => [
				[
					'method' => 'POST',
					'uri' => self::makeUri( '/oauth2/client' ),
					'postParams' => array_merge( self::DEFAULT_POST_PARAMS, self::POST_PARAMS_OWNERS_ONLY_RESTRICTION ),
					'headers' => [
						'Content-Type' => 'application/json'
					],
				],
				[
					'statusCode' => 400,
					'reasonPhrase' => 'Bad Request',
					'protocolVersion' => '1.1'
				],
				static function () {
					$user = User::createNew( 'RequestClientTestUser4' );
					$user->setEmail( 'test@test.com' );

					return $user;
				}
			],
			'Email Mismatch' => [
				[
					'method' => 'POST',
					'uri' => self::makeUri( '/oauth2/client' ),
					'postParams' => array_merge( self::DEFAULT_POST_PARAMS, self::POST_PARAMS_EMAIL_MISTMATCH ),
					'headers' => [
						'Content-Type' => 'application/json'
					],
				],
				[
					'statusCode' => 400,
					'reasonPhrase' => 'Bad Request',
					'protocolVersion' => '1.1'
				],
				static function () {
					$user = User::createNew( 'RequestClientTestUser5' );
					$user->setEmail( 'test@test.com' );

					return $user;
				}
			],
			'Invalid Grant Types for OwnerOnly client' => [
				[
					'method' => 'POST',
					'uri' => self::makeUri( '/oauth2/client' ),
					'postParams' => array_merge( self::DEFAULT_POST_PARAMS, self::POST_PARAMS_WRONG_GRANT_TYPES ),
					'headers' => [
						'Content-Type' => 'application/json'
					],
				],
				[
					'statusCode' => 400,
					'reasonPhrase' => 'Bad Request',
					'protocolVersion' => '1.1'
				],
				static function () {
					$user = User::createNew( 'RequestClientTestUser6' );
					$user->setEmail( 'test@test.com' );

					return $user;
				}
			],
			'Successful request' => [
				[
					'method' => 'POST',
					'uri' => self::makeUri( '/oauth2/client' ),
					'postParams' => self::DEFAULT_POST_PARAMS,
					'headers' => [
						'Content-Type' => 'application/json'
					],
				],
				[
					'statusCode' => 200,
					'reasonPhrase' => 'OK',
					'protocolVersion' => '1.1'
				],
				static function () {
					$user = User::createNew( 'RequestClientTestUser2' );
					$user->setEmail( 'test@test.com' );

					return $user;
				}
			],
			'Successful request owner only' => [
				[
					'method' => 'POST',
					'uri' => self::makeUri( '/oauth2/client' ),
					'postParams' => array_merge( self::DEFAULT_POST_PARAMS, self::POST_PARAMS_OWNERS_ONLY ),
					'headers' => [
						'Content-Type' => 'application/json'
					],
				],
				[
					'statusCode' => 200,
					'reasonPhrase' => 'OK',
					'protocolVersion' => '1.1',
				],
				static function () {
					$user = User::createNew( 'RequestClientTestUser10' );
					$user->setEmail( 'test@test.com' );

					return $user;
				},
				static function ( MediaWikiIntegrationTestCase $testCase, ResponseInterface $response ) {
					$responseBody = FormatJson::decode(
						$response->getBody()->getContents(),
						true
					);
					$testCase->assertArrayHasKey( 'access_token', $responseBody );
					$testCase->assertMatchesRegularExpression( '/((.*)\.(.*)\.(.*))/', $responseBody['access_token'] );
				},
			],
			'Successful scopes values' => [
				[
					'method' => 'POST',
					'uri' => self::makeUri( '/oauth2/client' ),
					'postParams' => [ 'scopes' => 'basic' ] + self::DEFAULT_POST_PARAMS,
					'headers' => [
						'Content-Type' => 'application/json'
					],
				],
				[
					'statusCode' => 200,
					'reasonPhrase' => 'OK',
					'protocolVersion' => '1.1'
				],
				static function () {
					$user = User::createNew( 'RequestClientTestUser7' );
					$user->setEmail( 'test@test.com' );

					return $user;
				}
			],
			'Scope with mwoauth-authonly' => [
				[
					'method' => 'POST',
					'uri' => self::makeUri( '/oauth2/client' ),
					'postParams' => [ 'scopes' => 'mwoauth-authonly' ] + self::DEFAULT_POST_PARAMS,
					'headers' => [
						'Content-Type' => 'application/json'
					],
				],
				[
					'statusCode' => 200,
					'reasonPhrase' => 'OK',
					'protocolVersion' => '1.1'
				],
				static function () {
					$user = User::createNew( 'RequestClientTestUser8' );
					$user->setEmail( 'test@test.com' );

					return $user;
				}
			],
			'Scope with mwoauth-authonlyprivate' => [
				[
					'method' => 'POST',
					'uri' => self::makeUri( '/oauth2/client' ),
					'postParams' => [ 'scopes' => 'mwoauth-authonlyprivate' ] + self::DEFAULT_POST_PARAMS,
					'headers' => [
						'Content-Type' => 'application/json'
					],
				],
				[
					'statusCode' => 200,
					'reasonPhrase' => 'OK',
					'protocolVersion' => '1.1'
				],
				static function () {
					$user = User::createNew( 'RequestClientTestUser9' );
					$user->setEmail( 'test@test.com' );

					return $user;
				}
			],
			'Failed scopes values' => [
				[
					'method' => 'POST',
					'uri' => self::makeUri( '/oauth2/client' ),
					'postParams' => [ 'scopes' => 'wrong' ] + self::DEFAULT_POST_PARAMS,
					'headers' => [
						'Content-Type' => 'application/json'
					],
				],
				[
					'statusCode' => 400,
					'reasonPhrase' => 'Bad Request',
					'protocolVersion' => '1.1'
				],
			],
		];
	}

	protected function newHandler(): Handler {
		return new RequestClient();
	}
}
