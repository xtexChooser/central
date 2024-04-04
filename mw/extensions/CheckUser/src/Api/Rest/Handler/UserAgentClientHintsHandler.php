<?php

namespace MediaWiki\CheckUser\Api\Rest\Handler;

use Config;
use MediaWiki\CheckUser\ClientHints\ClientHintsData;
use MediaWiki\CheckUser\Services\UserAgentClientHintsManager;
use MediaWiki\Rest\LocalizedHttpException;
use MediaWiki\Rest\SimpleHandler;
use MediaWiki\Rest\TokenAwareHandlerTrait;
use MediaWiki\Rest\Validator\JsonBodyValidator;
use MediaWiki\Rest\Validator\UnsupportedContentTypeBodyValidator;
use MediaWiki\Rest\Validator\Validator;
use MediaWiki\Revision\RevisionRecord;
use MediaWiki\Revision\RevisionStore;
use TypeError;
use Wikimedia\Message\MessageValue;
use Wikimedia\ParamValidator\ParamValidator;
use Wikimedia\Timestamp\ConvertibleTimestamp;

/**
 * Handler for POST requests to /checkuser/v0/useragent-clienthints/{type}/{id}
 *
 * Intended to be called by the ext.checkUser.clientHints ResourceLoader module,
 * in response to 'postEdit' mw.hook events.
 *
 * Eventually we can also use this endpoint for mapping data to CheckUser log events
 * in cu_log_event and cu_private_event.
 */
class UserAgentClientHintsHandler extends SimpleHandler {
	use TokenAwareHandlerTrait;

	private Config $config;
	private RevisionStore $revisionStore;
	private UserAgentClientHintsManager $userAgentClientHintsManager;

	/**
	 * @param Config $config
	 * @param RevisionStore $revisionStore
	 * @param UserAgentClientHintsManager $userAgentClientHintsManager
	 */
	public function __construct(
		Config $config, RevisionStore $revisionStore, UserAgentClientHintsManager $userAgentClientHintsManager
	) {
		$this->config = $config;
		$this->revisionStore = $revisionStore;
		$this->userAgentClientHintsManager = $userAgentClientHintsManager;
	}

	/**
	 * @inheritDoc
	 */
	public function validate( Validator $restValidator ): void {
		parent::validate( $restValidator );
		// Allow anonymous token needs to be true as logged out users can make requests to
		// this endpoint via the ext.checkUser.clientHints ResourceLoader module.
		$this->validateToken( true );
	}

	public function run() {
		if ( !$this->config->get( 'CheckUserClientHintsEnabled' ) ) {
			// Pretend the route doesn't exist if the feature flag is off.
			throw new LocalizedHttpException(
				new MessageValue( 'rest-no-match' ), 404
			);
		}
		$data = $this->getValidatedBody();
		// $data should be an array, but can be null when validation
		// failed and/or when the content type was form data.
		if ( !is_array( $data ) ) {
			// Taken from Validator::validateBody
			[ $contentType ] = explode( ';', $this->getRequest()->getHeaderLine( 'Content-Type' ), 2 );
			$contentType = strtolower( trim( $contentType ) );
			if ( $contentType !== 'application/json' ) {
				// Same exception as used in UnsupportedContentTypeBodyValidator
				throw new LocalizedHttpException(
					new MessageValue( 'rest-unsupported-content-type', [ $contentType ] ),
					415
				);
			} else {
				// Should be caught by JsonBodyValidator::validateBody, but if this
				// point is reached a non-array still indicates a problem with the
				// data submitted by the client and thus a 400 error is appropriate.
				throw new LocalizedHttpException( new MessageValue( 'rest-bad-json-body' ), 400 );
			}
		}
		try {
			// ::newFromJsApi with the $data may raise a TypeError as the $data
			// does not have its type validated (T305973).
			$clientHints = ClientHintsData::newFromJsApi( $data );
		} catch ( TypeError $e ) {
			throw new LocalizedHttpException( new MessageValue( 'rest-bad-json-body' ), 400 );
		}
		$type = $this->getValidatedParams()['type'];
		$identifier = $this->getValidatedParams()['id'];
		if ( $type === 'revision' ) {
			$this->performValidationForRevision( $identifier );
		} else {
			// If the type is not supported, pretend the route doesn't exist.
			throw new LocalizedHttpException(
				new MessageValue( 'rest-no-match' ), 404
			);
		}
		$status = $this->userAgentClientHintsManager->insertClientHintValues( $clientHints, $identifier, $type );
		if ( !$status->isGood() ) {
			$error = $status->getErrors()[0];
			// A client hints mapping entry already exists.
			throw new LocalizedHttpException(
				new MessageValue( $error['message'], $error['params'][0] ),
				400
			);
		}

		$response = $this->getResponseFactory()->createJson( [
			"value" => $this->getResponseFactory()->formatMessage(
				new MessageValue( 'checkuser-api-useragent-clienthints-explanation' )
			)
		] );
		return $response;
	}

	/**
	 * Check whether Client Hints data can be stored for the given revision ID.
	 * This method checks that the revision with this ID exists, was not made
	 * over wgCheckUserClientHintsRestApiMaxTimeLag seconds ago, and that the
	 * user making the request made the edit with this ID.
	 *
	 * @param int $revisionId The revision ID
	 * @return void
	 * @throws LocalizedHttpException If the checks fail, this exception will be raised.
	 */
	private function performValidationForRevision( int $revisionId ) {
		// Check the revision exists.
		$revision = $this->revisionStore->getRevisionById( $revisionId );
		if ( !$revision ) {
			throw new LocalizedHttpException(
				new MessageValue( 'rest-nonexistent-revision', [ $revisionId ] ), 404 );
		}
		$this->performTimestampValidation( $revision->getTimestamp(), 'revision', $revisionId );
		// Check the performer of the action is the same as the user submitting this REST API request
		$user = $this->getAuthority()->getUser();
		if (
			!$revision->getUser( RevisionRecord::RAW ) ||
			!$revision->getUser( RevisionRecord::RAW )->equals( $user )
		) {
			throw new LocalizedHttpException(
				new MessageValue(
					'checkuser-api-useragent-clienthints-revision-user-mismatch',
					[ $user->getId(), $revisionId ]
				),
				401
			);
		}
	}

	/**
	 * Validate that the API was not called over wgCheckUserClientHintsRestApiMaxTimeLag
	 * seconds ago.
	 *
	 * @param ?string $associatedEntryTimestamp The timestamp associated with the $identifier in TS_MW form.
	 *   If null, the validation will always fail.
	 * @param string $type The type of the $identifier (e.g. revision)
	 * @param int $identifier The ID of the entry of type $type
	 * @return void
	 * @throws LocalizedHttpException If the validation fails, this exception will be raised.
	 */
	private function performTimestampValidation(
		?string $associatedEntryTimestamp, string $type, int $identifier
	): void {
		// Check that the API was not called too long after the edit
		$cutoff = ConvertibleTimestamp::convert(
			TS_MW,
			ConvertibleTimestamp::time() - $this->config->get( 'CheckUserClientHintsRestApiMaxTimeLag' )
		);
		// If there is no timestamp associated with this action, then
		// this method cannot perform the timestamp check. This should
		// rarely happen and is likely to occur for actions that are
		// already too old, so just don't store data in this case.
		if ( $associatedEntryTimestamp < $cutoff ) {
			throw new LocalizedHttpException(
				new MessageValue(
					'checkuser-api-useragent-clienthints-called-too-late',
					[ $type, $identifier ]
				),
				403
			);
		}
	}

	/** @inheritDoc */
	public function getBodyValidator( $contentType ) {
		if ( $contentType !== 'application/json' ) {
			return new UnsupportedContentTypeBodyValidator( $contentType );
		}

		// These are always sent by the browser, so mark as required.
		$lowEntropyClientHints = [
			'brands' => [],
			'mobile' => [
				ParamValidator::PARAM_TYPE => 'bool',
			],
			'platform' => [
				ParamValidator::PARAM_TYPE => 'string',
			],
		];

		$highEntropyClientHints = [
			'architecture' => [
				ParamValidator::PARAM_TYPE => 'string'
			],
			'bitness' => [
				ParamValidator::PARAM_TYPE => 'string'
			],
			'fullVersionList' => [],
			'model' => [
				ParamValidator::PARAM_TYPE => 'string',
			],
			'platformVersion' => [
				ParamValidator::PARAM_TYPE => 'string',
			]
		];
		$expectedJsonStructure = array_merge(
			$lowEntropyClientHints,
			$highEntropyClientHints,
			$this->getTokenParamDefinition()
		);
		return new JsonBodyValidator( $expectedJsonStructure );
	}

	public function needsWriteAccess() {
		return true;
	}

	/** @inheritDoc */
	public function getParamSettings() {
		return [
			'type' => [
				self::PARAM_SOURCE => 'path',
				ParamValidator::PARAM_TYPE => UserAgentClientHintsManager::SUPPORTED_TYPES,
				ParamValidator::PARAM_REQUIRED => true,
			],
			'id' => [
				self::PARAM_SOURCE => 'path',
				ParamValidator::PARAM_TYPE => 'integer',
				ParamValidator::PARAM_REQUIRED => true,
			]
		];
	}
}
