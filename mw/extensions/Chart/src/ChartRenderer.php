<?php

namespace MediaWiki\Extension\Chart;

use MediaWiki\Config\ServiceOptions;
use MediaWiki\Context\RequestContext;
use MediaWiki\Http\HttpRequestFactory;
use MediaWiki\Language\FormatterFactory;
use MediaWiki\Shell\Shell;
use MediaWiki\Status\Status;
use MWCryptHash;
use Psr\Log\LoggerInterface;
use stdclass;

class ChartRenderer {

	private ServiceOptions $options;
	private HttpRequestFactory $httpRequestFactory;
	private FormatterFactory $formatterFactory;
	private LoggerInterface $logger;

	/**
	 * @internal For use by ServiceWiring
	 */
	public const CONSTRUCTOR_OPTIONS = [
		'ChartServiceUrl',
		'ChartCliPath'
	];

	/**
	 * @param ServiceOptions $options
	 * @param HttpRequestFactory $httpRequestFactory
	 * @param FormatterFactory $formatterFactory
	 * @param LoggerInterface $logger
	 */
	public function __construct(
		ServiceOptions $options,
		HttpRequestFactory $httpRequestFactory,
		FormatterFactory $formatterFactory,
		LoggerInterface $logger
	) {
		$options->assertRequiredOptions( self::CONSTRUCTOR_OPTIONS );
		$this->options = $options;
		$this->httpRequestFactory = $httpRequestFactory;
		$this->formatterFactory = $formatterFactory;
		$this->logger = $logger;
	}

	/**
	 * Render a chart from a definition object and a tabular data object.
	 *
	 * @param stdclass $chartDef Chart definition, obtained from JCChartContent::getContent()
	 * @param stdclass $tabularData Tabular data, obtained from JCTabularContent::getContent()
	 * @param array{width?:string,height?:string} $options Additional rendering options:
	 *   'width': Width of the chart, in pixels. Overrides width specified in the chart definition
	 *   'height': Height of the chart, in pixels. Overrides height specified in the chart definition.
	 * @return Status A Status object wrapping an SVG string or an error
	 */
	public function renderSVG( stdclass $chartDef, stdclass $tabularData, array $options = [] ): Status {
		// Prefix for IDs in the SVG. This has to be unique between charts on the same page, to
		// prevent ID collisions (T371558). If the same chart with the same data is displayed twice
		// on the same page, this gives them the same ID prefixes and causes their IDs to collide,
		// but that doesn't seem to cause a problem in practice.
		$definitionForHash = json_encode( [ 'format' => $chartDef, 'source' => $tabularData ] );
		$chartDef = clone $chartDef;
		$chartDef->idPrefix = 'mw-chart-' . MWCryptHash::hash( $definitionForHash, false );

		if ( $this->options->get( 'ChartServiceUrl' ) !== null ) {
			return $this->renderWithService( $chartDef, $tabularData, $options );
		}

		return $this->renderWithCli( $chartDef, $tabularData, $options );
	}

	private function renderWithService( stdclass $chartDef, stdclass $tabularData, array $options ): Status {
		$requestData = array_merge( [
			'definition' => $chartDef,
			'data' => $tabularData,
		], $options );

		$requestOptions = [
			'method' => 'POST',
			'postData' => json_encode( $requestData )
		];
		$request = $this->httpRequestFactory->create(
			$this->options->get( 'ChartServiceUrl' ),
			$requestOptions,
			__METHOD__
		);
		$request->setHeader( 'Content-Type', 'application/json' );

		$status = $request->execute();
		if ( !$status->isOK() ) {
			[ $message, $context ] = $this->formatterFactory->getStatusFormatter( RequestContext::getMain() )
				->getPsr3MessageAndContext( $status );
			$this->logger->error(
				'Chart service request returned error: {error}',
				[ 'error' => $message ] + $context
			);
			return Status::newFatal( 'chart-error-rendering-error' );
		}
		$response = $request->getContent();
		return Status::newGood( $response );
	}

	private function renderWithCli( stdclass $chartDef, stdclass $tabularData, array $options ): Status {
		if ( Shell::isDisabled() ) {
			return Status::newFatal( 'chart-error-shell-disabled' );
		}

		$dataPath = tempnam( \wfTempDir(), 'data-json' );
		file_put_contents( $dataPath, json_encode( [
			'definition' => $chartDef,
			'data' => $tabularData,
			...$options
		] ) );

		$result = Shell::command(
			'node',
			$this->options->get( 'ChartCliPath' ),
			$dataPath,
			'-'
		 )
			->execute();

		$error = $result->getStderr();
		if ( $error ) {
			$this->logger->error(
				'Chart shell command returned error: {error}',
				[ 'error' => $error ]
			);

			// @todo tracking category
			$status = Status::newFatal( 'chart-error-rendering-error' );
		} else {
			$svg = $result->getStdout();
			$status = Status::newGood( $svg );
		}

		unlink( $dataPath );
		return $status;
	}
}
