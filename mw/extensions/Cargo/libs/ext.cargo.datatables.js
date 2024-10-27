$(document).ready(function() {
	$('.cargoDynamicTable').each( function( index ) {
		var params = {};
		var pageLength = $(this).attr( 'data-page-length' );
		if ( pageLength != '' && pageLength > 0 && parseInt( pageLength ) == pageLength ) {
			pageLength = parseInt( pageLength );
			params['pageLength'] = pageLength;
			var lengthOptions = [ 10, 25, 50, 100 ];
			// If this is not one of the default options, add it
			// to the list.
			if ( lengthOptions.indexOf( pageLength ) < 0 ) {
				lengthOptions.push( pageLength );
				lengthOptions.sort( function(a, b){return a-b;} );
				params['lengthMenu'] = lengthOptions;
			}
		}
		params[ 'aoColumns' ] = [];
		var detailsFields = $(this).attr( 'data-details-fields' );
		if ( detailsFields ) {
			params['columnDefs'] = [{ "orderable":false, "targets": 0 }];
			params[ 'aoColumns' ].push( { 'sWidth': '5px' } );
		}
		// Specify column widths if provided in display parameters
		var columnWidths = $( this ).attr( 'data-widths' );
		var columns  = [];
		$( this ).find( 'th' ).each( function () {
			if ( $( this ).text() !== "" )
				columns.push( $( this ).text() );
		} );
		columns = [...new Set(columns)];
		if ( columnWidths !== undefined ) {
			columnWidths = columnWidths.split( ',' );
			params[ 'bAutoWidth' ] = false;
			columnWidths.forEach( function ( value ) {
				columns.shift();
				params[ 'aoColumns' ].push( {
					'sWidth': value.trim()
				} );
			} );
		}
		while ( columns.length !== 0 ) {
			params[ 'aoColumns' ].push( {
				'sWidth': 'auto'
			} );
			columns.shift();
		}
		$( this ).attr( 'style', 'table-layout: fixed; word-wrap:break-word;' );
		var table = $(this).DataTable( params );

		// searchable columns
		var tfoot = $(this).find('tfoot');
		$(this).find('tfoot th').each( function () {
			var placeholder = $(this).data('placeholder');
			if ( placeholder ) {
				$(this).html( '<input type="text" placeholder="'+placeholder+'"/>' );
				tfoot.find('th').css('border-top', 'none');
				tfoot.css('display', 'table-header-group');
			}
		} );
		table.columns().every( function () {
			var that = this;
			$( 'input', this.footer() ).on( 'keyup change', function () {
				if ( that.search() !== this.value ) {
					that.search( this.value )
						.draw();
				}
			} );
		} );

		// hidden fields
		$( 'a.toggle-vis' ).each( function () {
			var column = table.column( $(this).data( 'column' ) );
			column.visible( false );
			$( this ).on( 'click', function ( e ) {
				e.preventDefault();
				column.visible( ! column.visible() );
			} );
		} );

		// Add event listener for opening and closing details
		$(this).find('tbody').on('click', 'td.details-control', function () {
			var tr = $(this).closest('tr');
			var row = table.row( tr );
			if ( row.child.isShown() ) {
				// This row is already open - close it
				row.child.hide();
				tr.removeClass('shown');
			} else {
				// Open this row
				row.child( tr.data('details') ).show();
				tr.addClass('shown');
			}
		} );

		// Add popup tooltip for all column headers
		var headerTooltips = $( this ).attr( 'data-tooltips' );
		if ( headerTooltips !== undefined ) {
			headerTooltips = headerTooltips.split( ',' );
			$( 'th[aria-controls=DataTables_Table_' + index + ']' ).each( function ( idx ) {
				if ( headerTooltips[idx] != undefined && headerTooltips[idx].trim() != '' ) {
					var popupButton = new OO.ui.PopupButtonWidget( {
						icon: 'info',
						framed: false,
						popup: {
							$content: $('<p>' + headerTooltips[ idx ].trim() + '</p>')
						}
					} );
					$( this ).append( '&nbsp;' );
					$( this ).append( popupButton.$element );
				}
			} );
			$( '.oo-ui-popupWidget-body p' ).attr( 'style', 'font-weight: normal;' );
		}
	} );

} );
