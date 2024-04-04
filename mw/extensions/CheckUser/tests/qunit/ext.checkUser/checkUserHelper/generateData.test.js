'use strict';

const generateData = require( '../../../../modules/ext.checkUser/checkuser/checkUserHelper/generateData.js' );

QUnit.module( 'ext.checkUser.checkUserHelper.generateData', QUnit.newMwEnvironment() );

QUnit.test( 'Test that generateData returns the expected data', function ( assert ) {
	const cases = require( './cases/generateData.json' );

	cases.forEach( function ( caseItem ) {
		let html = '<div id="checkuserresults"><ul>';
		caseItem.items.forEach( ( resultLine ) => {
			html += '<li>';
			html += '<span class="mw-checkuser-user-link">' + resultLine.userLink + '</span>';
			html += '<span class="mw-checkuser-agent">' + resultLine.userAgent + '</span>';
			html += '<span class="mw-checkuser-ip">' + resultLine.IP + '</span>';
			if ( resultLine.XFFTrusted ) {
				html += '<span class="mw-checkuser-xff mw-checkuser-xff-trusted">';
			} else {
				html += '<span class="mw-checkuser-xff">';
			}
			html += resultLine.XFF + '</span>';
			html += '</li>';
		} );
		html += '</ul></div>';
		// eslint-disable-next-line no-jquery/no-global-selector
		$( '#qunit-fixture' ).html( html );
		generateData().then( ( data ) => {
			assert.deepEqual(
				data,
				caseItem.expectedData,
				caseItem.msg
			);
		} );
	} );
} );
