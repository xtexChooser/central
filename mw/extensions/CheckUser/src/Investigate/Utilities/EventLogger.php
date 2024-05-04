<?php

namespace MediaWiki\CheckUser\Investigate\Utilities;

use ExtensionRegistry;
use MediaWiki\Extension\EventLogging\EventLogging;

class EventLogger {
	private ExtensionRegistry $extensionRegistry;

	/**
	 * @param ExtensionRegistry $extensionRegistry
	 */
	public function __construct(
		ExtensionRegistry $extensionRegistry
	) {
		$this->extensionRegistry = $extensionRegistry;
	}

	/**
	 * If the EventLogging extension is loaded, then submit an analytics event to the event
	 * ingestion service.
	 *
	 * The event will be validated using the /analytics/legacy/specialinvestigate schema.
	 *
	 * @param array $event
	 */
	public function logEvent( $event ): void {
		if ( $this->extensionRegistry->isLoaded( 'EventLogging' ) ) {
			EventLogging::submit(
				'eventlogging_SpecialInvestigate',
				[
					'$schema' => '/analytics/legacy/specialinvestigate/1.0.0',
					'event' => $event,
				]
			);
		}
	}

	/**
	 * Get a timestamp in milliseconds.
	 *
	 * @return int
	 */
	public function getTime(): int {
		return (int)round( microtime( true ) * 1000 );
	}
}
