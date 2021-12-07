/* Detect-zoom
 * -----------
 * Cross Browser Zoom and Pixel Ratio Detector
 * Version 1.0.4 | Apr 1 2013
 * dual-licensed under the WTFPL and MIT license
 * Maintained by https://github/tombigel
 * Original developer https://github.com/yonran
 */

//AMD and CommonJS initialization copied from https://github.com/zohararad/audio5js
(function (root, ns, factory) {
    "use strict";

    if (typeof (module) !== 'undefined' && module.exports) { // CommonJS
        module.exports = factory(ns, root);
    } else if (typeof (define) === 'function' && define.amd) { // AMD
        define("factory", function () {
            return factory(ns, root);
        });
    } else {
        root[ns] = factory(ns, root);
    }

}(window, 'detectZoom', function () {

    /**
     * Use devicePixelRatio if supported by the browser
     * @return {Number}
     * @private
     */
    var devicePixelRatio = function () {
        return window.devicePixelRatio || 1;
    };

    /**
     * Fallback function to set default values
     * @return {Object}
     * @private
     */
    var fallback = function () {
        return {
            zoom: 1,
            devicePxPerCssPx: 1
        };
    };
    /**
     * IE 8 and 9: no trick needed!
     * TODO: Test on IE10 and Windows 8 RT
     * @return {Object}
     * @private
     **/
    var ie8 = function () {
        var zoom = Math.round((screen.deviceXDPI / screen.logicalXDPI) * 100) / 100;
        return {
            zoom: zoom,
            devicePxPerCssPx: zoom * devicePixelRatio()
        };
    };

    /**
     * For IE10 we need to change our technique again...
     * thanks https://github.com/stefanvanburen
     * @return {Object}
     * @private
     */
    var ie10 = function () {
        var zoom = Math.round((document.documentElement.offsetHeight / window.innerHeight) * 100) / 100;
        return {
            zoom: zoom,
            devicePxPerCssPx: zoom * devicePixelRatio()
        };
    };

    /**
     * Mobile WebKit
     * the trick: window.innerWIdth is in CSS pixels, while
     * screen.width and screen.height are in system pixels.
     * And there are no scrollbars to mess up the measurement.
     * @return {Object}
     * @private
     */
    var webkitMobile = function () {
        var deviceWidth = (Math.abs(window.orientation) == 90) ? screen.height : screen.width;
        var zoom = deviceWidth / window.innerWidth;
        return {
            zoom: zoom,
            devicePxPerCssPx: zoom * devicePixelRatio()
        };
    };

    /**
     * Desktop Webkit
     * the trick: an element's clientHeight is in CSS pixels, while you can
     * set its line-height in system pixels using font-size and
     * -webkit-text-size-adjust:none.
     * device-pixel-ratio: http://www.webkit.org/blog/55/high-dpi-web-sites/
     *
     * Previous trick (used before http://trac.webkit.org/changeset/100847):
     * documentElement.scrollWidth is in CSS pixels, while
     * document.width was in system pixels. Note that this is the
     * layout width of the document, which is slightly different from viewport
     * because document width does not include scrollbars and might be wider
     * due to big elements.
     * @return {Object}
     * @private
     */
    var webkit = function () {
        var important = function (str) {
            return str.replace(/;/g, " !important;");
        };

        var div = document.createElement('div');
        div.innerHTML = "1<br>2<br>3<br>4<br>5<br>6<br>7<br>8<br>9<br>0";
        div.setAttribute('style', important('font: 100px/1em sans-serif; -webkit-text-size-adjust: none; text-size-adjust: none; height: auto; width: 1em; padding: 0; overflow: visible;'));

        // The container exists so that the div will be laid out in its own flow
        // while not impacting the layout, viewport size, or display of the
        // webpage as a whole.
        // Add !important and relevant CSS rule resets
        // so that other rules cannot affect the results.
        var container = document.createElement('div');
        container.setAttribute('style', important('width:0; height:0; overflow:hidden; visibility:hidden; position: absolute;'));
        container.appendChild(div);

        document.body.appendChild(container);
        var zoom = 1000 / div.clientHeight;
        zoom = Math.round(zoom * 100) / 100;
        document.body.removeChild(container);

        return{
            zoom: zoom,
            devicePxPerCssPx: zoom * devicePixelRatio()
        };
    };

    /**
     * no real trick; device-pixel-ratio is the ratio of device dpi / css dpi.
     * (Note that this is a different interpretation than Webkit's device
     * pixel ratio, which is the ratio device dpi / system dpi).
     *
     * Also, for Mozilla, there is no difference between the zoom factor and the device ratio.
     *
     * @return {Object}
     * @private
     */
    var firefox4 = function () {
        var zoom = mediaQueryBinarySearch('min--moz-device-pixel-ratio', '', 0, 10, 20, 0.0001);
        zoom = Math.round(zoom * 100) / 100;
        return {
            zoom: zoom,
            devicePxPerCssPx: zoom
        };
    };

    /**
     * Firefox 18.x
     * Mozilla added support for devicePixelRatio to Firefox 18,
     * but it is affected by the zoom level, so, like in older
     * Firefox we can't tell if we are in zoom mode or in a device
     * with a different pixel ratio
     * @return {Object}
     * @private
     */
    var firefox18 = function () {
        return {
            zoom: firefox4().zoom,
            devicePxPerCssPx: devicePixelRatio()
        };
    };

    /**
     * works starting Opera 11.11
     * the trick: outerWidth is the viewport width including scrollbars in
     * system px, while innerWidth is the viewport width including scrollbars
     * in CSS px
     * @return {Object}
     * @private
     */
    var opera11 = function () {
        var zoom = window.top.outerWidth / window.top.innerWidth;
        zoom = Math.round(zoom * 100) / 100;
        return {
            zoom: zoom,
            devicePxPerCssPx: zoom * devicePixelRatio()
        };
    };

    /**
     * Use a binary search through media queries to find zoom level in Firefox
     * @param property
     * @param unit
     * @param a
     * @param b
     * @param maxIter
     * @param epsilon
     * @return {Number}
     */
    var mediaQueryBinarySearch = function (property, unit, a, b, maxIter, epsilon) {
        var matchMedia;
        var head, style, div;
        if (window.matchMedia) {
            matchMedia = window.matchMedia;
        } else {
            head = document.getElementsByTagName('head')[0];
            style = document.createElement('style');
            head.appendChild(style);

            div = document.createElement('div');
            div.className = 'mediaQueryBinarySearch';
            div.style.display = 'none';
            document.body.appendChild(div);

            matchMedia = function (query) {
                style.sheet.insertRule('@media ' + query + '{.mediaQueryBinarySearch ' + '{text-decoration: underline} }', 0);
                var matched = getComputedStyle(div, null).textDecoration == 'underline';
                style.sheet.deleteRule(0);
                return {matches: matched};
            };
        }
        var ratio = binarySearch(a, b, maxIter);
        if (div) {
            head.removeChild(style);
            document.body.removeChild(div);
        }
        return ratio;

        function binarySearch(a, b, maxIter) {
            var mid = (a + b) / 2;
            if (maxIter <= 0 || b - a < epsilon) {
                return mid;
            }
            var query = "(" + property + ":" + mid + unit + ")";
            if (matchMedia(query).matches) {
                return binarySearch(mid, b, maxIter - 1);
            } else {
                return binarySearch(a, mid, maxIter - 1);
            }
        }
    };

    /**
     * Generate detection function
     * @private
     */
    var detectFunction = (function () {
        var func = fallback;
        //IE8+
        if (!isNaN(screen.logicalXDPI) && !isNaN(screen.systemXDPI)) {
            func = ie8;
        }
        // IE10+ / Touch
        else if (window.navigator.msMaxTouchPoints) {
            func = ie10;
        }
        //Mobile Webkit
        else if ('orientation' in window && typeof document.body.style.webkitMarquee === 'string') {
            func = webkitMobile;
        }
        //WebKit
        else if (typeof document.body.style.webkitMarquee === 'string') {
            func = webkit;
        }
        //Opera
        else if (navigator.userAgent.indexOf('Opera') >= 0) {
            func = opera11;
        }
        //Last one is Firefox
        //FF 18.x
        else if (window.devicePixelRatio) {
            func = firefox18;
        }
        //FF 4.0 - 17.x
        else if (firefox4().zoom > 0.001) {
            func = firefox4;
        }

        return func;
    }());


    return ({

        /**
         * Ratios.zoom shorthand
         * @return {Number} Zoom level
         */
        zoom: function () {
            return detectFunction().zoom;
        },

        /**
         * Ratios.devicePxPerCssPx shorthand
         * @return {Number} devicePxPerCssPx level
         */
        device: function () {
            return detectFunction().devicePxPerCssPx;
        }
    });
}));

var wpcom_img_zoomer = {
        clientHintSupport: {
                gravatar: false,
                files: false,
                photon: false,
                mshots: false,
                staticAssets: false,
                latex: false,
                imgpress: false,
        },
	useHints: false,
	zoomed: false,
	timer: null,
	interval: 1000, // zoom polling interval in millisecond

	// Should we apply width/height attributes to control the image size?
	imgNeedsSizeAtts: function( img ) {
		// Do not overwrite existing width/height attributes.
		if ( img.getAttribute('width') !== null || img.getAttribute('height') !== null )
			return false;
		// Do not apply the attributes if the image is already constrained by a parent element.
		if ( img.width < img.naturalWidth || img.height < img.naturalHeight )
			return false;
		return true;
	},

        hintsFor: function( service ) {
                if ( this.useHints === false ) {
                        return false;
                }
                if ( this.hints() === false ) {
                        return false;
                }
                if ( typeof this.clientHintSupport[service] === "undefined" ) {
                        return false;
                }
                if ( this.clientHintSupport[service] === true ) {
                        return true;
                }
                return false;
        },

	hints: function() {
		try {
			var chrome = window.navigator.userAgent.match(/\sChrome\/([0-9]+)\.[.0-9]+\s/)
			if (chrome !== null) {
				var version = parseInt(chrome[1], 10)
				if (isNaN(version) === false && version >= 46) {
					return true
				}
			}
		} catch (e) {
			return false
		}
		return false
	},

	init: function() {
		var t = this;
		try{
			t.zoomImages();
			t.timer = setInterval( function() { t.zoomImages(); }, t.interval );
		}
		catch(e){
		}
	},

	stop: function() {
		if ( this.timer )
			clearInterval( this.timer );
	},

	getScale: function() {
		var scale = detectZoom.device();
		// Round up to 1.5 or the next integer below the cap.
		if      ( scale <= 1.0 ) scale = 1.0;
		else if ( scale <= 1.5 ) scale = 1.5;
		else if ( scale <= 2.0 ) scale = 2.0;
		else if ( scale <= 3.0 ) scale = 3.0;
		else if ( scale <= 4.0 ) scale = 4.0;
		else                     scale = 5.0;
		return scale;
	},

	shouldZoom: function( scale ) {
		var t = this;
		// Do not operate on hidden frames.
		if ( "innerWidth" in window && !window.innerWidth )
			return false;
		// Don't do anything until scale > 1
		if ( scale == 1.0 && t.zoomed == false )
			return false;
		return true;
	},

	zoomImages: function() {
		var t = this;
		var scale = t.getScale();
		if ( ! t.shouldZoom( scale ) ){
			return;
		}
		t.zoomed = true;
		// Loop through all the <img> elements on the page.
		var imgs = document.getElementsByTagName("img");

		for ( var i = 0; i < imgs.length; i++ ) {
			// Wait for original images to load
			if ( "complete" in imgs[i] && ! imgs[i].complete )
				continue;

			// Skip images that have srcset attributes.
			if ( imgs[i].hasAttribute('srcset') ) {
				continue;
			}

			// Skip images that don't need processing.
			var imgScale = imgs[i].getAttribute("scale");
			if ( imgScale == scale || imgScale == "0" )
				continue;

			// Skip images that have already failed at this scale
			var scaleFail = imgs[i].getAttribute("scale-fail");
			if ( scaleFail && scaleFail <= scale )
				continue;

			// Skip images that have no dimensions yet.
			if ( ! ( imgs[i].width && imgs[i].height ) )
				continue;

			// Skip images from Lazy Load plugins
			if ( ! imgScale && imgs[i].getAttribute("data-lazy-src") && (imgs[i].getAttribute("data-lazy-src") !== imgs[i].getAttribute("src")))
				continue;

			if ( t.scaleImage( imgs[i], scale ) ) {
				// Mark the img as having been processed at this scale.
				imgs[i].setAttribute("scale", scale);
			}
			else {
				// Set the flag to skip this image.
				imgs[i].setAttribute("scale", "0");
			}
		}
	},

	scaleImage: function( img, scale ) {
		var t = this;
		var newSrc = img.src;

                var isFiles = false;
                var isLatex = false;
                var isPhoton = false;

		// Skip slideshow images
		if ( img.parentNode.className.match(/slideshow-slide/) )
			return false;

		// Skip CoBlocks Lightbox images
		if ( img.parentNode.className.match(/coblocks-lightbox__image/) )
			return false;

		// Scale gravatars that have ?s= or ?size=
		if ( img.src.match( /^https?:\/\/([^\/]*\.)?gravatar\.com\/.+[?&](s|size)=/ ) ) {
                        if ( this.hintsFor( "gravatar" ) === true ) {
                                return false;
                        }
			newSrc = img.src.replace( /([?&](s|size)=)(\d+)/, function( $0, $1, $2, $3 ) {
				// Stash the original size
				var originalAtt = "originals",
				originalSize = img.getAttribute(originalAtt);
				if ( originalSize === null ) {
					originalSize = $3;
					img.setAttribute(originalAtt, originalSize);
					if ( t.imgNeedsSizeAtts( img ) ) {
						// Fix width and height attributes to rendered dimensions.
						img.width = img.width;
						img.height = img.height;
					}
				}
				// Get the width/height of the image in CSS pixels
				var size = img.clientWidth;
				// Convert CSS pixels to device pixels
				var targetSize = Math.ceil(img.clientWidth * scale);
				// Don't go smaller than the original size
				targetSize = Math.max( targetSize, originalSize );
				// Don't go larger than the service supports
				targetSize = Math.min( targetSize, 512 );
				return $1 + targetSize;
			});
		}

		// Scale mshots that have width
		else if ( img.src.match(/^https?:\/\/([^\/]+\.)*(wordpress|wp)\.com\/mshots\/.+[?&]w=\d+/) ) {
                        if ( this.hintsFor( "mshots" ) === true ) {
                                return false;
                        }
			newSrc = img.src.replace( /([?&]w=)(\d+)/, function($0, $1, $2) {
				// Stash the original size
				var originalAtt = 'originalw', originalSize = img.getAttribute(originalAtt);
				if ( originalSize === null ) {
					originalSize = $2;
					img.setAttribute(originalAtt, originalSize);
					if ( t.imgNeedsSizeAtts( img ) ) {
						// Fix width and height attributes to rendered dimensions.
						img.width = img.width;
						img.height = img.height;
					}
				}
				// Get the width of the image in CSS pixels
				var size = img.clientWidth;
				// Convert CSS pixels to device pixels
				var targetSize = Math.ceil(size * scale);
				// Don't go smaller than the original size
				targetSize = Math.max( targetSize, originalSize );
				// Don't go bigger unless the current one is actually lacking
				if ( scale > img.getAttribute("scale") && targetSize <= img.naturalWidth )
					targetSize = $2;
				if ( $2 != targetSize )
					return $1 + targetSize;
				return $0;
			});

			// Update height attribute to match width
			newSrc = newSrc.replace( /([?&]h=)(\d+)/, function($0, $1, $2) {
				if ( newSrc == img.src ) {
					return $0;
				}
				// Stash the original size
				var originalAtt = 'originalh', originalSize = img.getAttribute(originalAtt);
				if ( originalSize === null ) {
					originalSize = $2;
					img.setAttribute(originalAtt, originalSize);
				}
				// Get the height of the image in CSS pixels
				var size = img.clientHeight;
				// Convert CSS pixels to device pixels
				var targetSize = Math.ceil(size * scale);
				// Don't go smaller than the original size
				targetSize = Math.max( targetSize, originalSize );
				// Don't go bigger unless the current one is actually lacking
				if ( scale > img.getAttribute("scale") && targetSize <= img.naturalHeight )
					targetSize = $2;
				if ( $2 != targetSize )
					return $1 + targetSize;
				return $0;
			});
		}

		// Scale simple imgpress queries (s0.wp.com) that only specify w/h/fit
		else if ( img.src.match(/^https?:\/\/([^\/.]+\.)*(wp|wordpress)\.com\/imgpress\?(.+)/) ) {
                        if ( this.hintsFor( "imgpress" ) === true ) {
                                return false; 
                        }
			var imgpressSafeFunctions = ["zoom", "url", "h", "w", "fit", "filter", "brightness", "contrast", "colorize", "smooth", "unsharpmask"];
			// Search the query string for unsupported functions.
			var qs = RegExp.$3.split('&');
			for ( var q in qs ) {
				q = qs[q].split('=')[0];
				if ( imgpressSafeFunctions.indexOf(q) == -1 ) {
					return false;
				}
			}
			// Fix width and height attributes to rendered dimensions.
			img.width = img.width;
			img.height = img.height;
			// Compute new src
			if ( scale == 1 )
				newSrc = img.src.replace(/\?(zoom=[^&]+&)?/, '?');
			else
				newSrc = img.src.replace(/\?(zoom=[^&]+&)?/, '?zoom=' + scale + '&');
		}

		// Scale files.wordpress.com, LaTeX, or Photon images (i#.wp.com)
		else if (
			( isFiles = img.src.match(/^https?:\/\/([^\/]+)\.files\.wordpress\.com\/.+[?&][wh]=/) ) ||
			( isLatex = img.src.match(/^https?:\/\/([^\/.]+\.)*(wp|wordpress)\.com\/latex\.php\?(latex|zoom)=(.+)/) ) ||
			( isPhoton = img.src.match(/^https?:\/\/i[\d]{1}\.wp\.com\/(.+)/) )
		) {
                        if ( false !== isFiles && this.hintsFor( "files" ) === true ) {
                                return false
                        }
                        if ( false !== isLatex && this.hintsFor( "latex" ) === true ) {
                                return false
                        }
                        if ( false !== isPhoton && this.hintsFor( "photon" ) === true ) {
                                return false
                        }
			// Fix width and height attributes to rendered dimensions.
			img.width = img.width;
			img.height = img.height;
			// Compute new src
			if ( scale == 1 ) {
				newSrc = img.src.replace(/\?(zoom=[^&]+&)?/, '?');
			} else {
				newSrc = img.src;

				var url_var = newSrc.match( /([?&]w=)(\d+)/ );
				if ( url_var !== null && url_var[2] ) {
					newSrc = newSrc.replace( url_var[0], url_var[1] + img.width );
				}

				url_var = newSrc.match( /([?&]h=)(\d+)/ );
				if ( url_var !== null && url_var[2] ) {
					newSrc = newSrc.replace( url_var[0], url_var[1] + img.height );
				}

				var zoom_arg = '&zoom=2';
				if ( !newSrc.match( /\?/ ) ) {
					zoom_arg = '?zoom=2';
				}
				img.setAttribute( 'srcset', newSrc + zoom_arg + ' ' + scale + 'x' );
			}
		}

		// Scale static assets that have a name matching *-1x.png or *@1x.png
		else if ( img.src.match(/^https?:\/\/[^\/]+\/.*[-@]([12])x\.(gif|jpeg|jpg|png)(\?|$)/) ) {
                        if ( this.hintsFor( "staticAssets" ) === true ) {
                                return false; 
                        }
			// Fix width and height attributes to rendered dimensions.
			img.width = img.width;
			img.height = img.height;
			var currentSize = RegExp.$1, newSize = currentSize;
			if ( scale <= 1 )
				newSize = 1;
			else
				newSize = 2;
			if ( currentSize != newSize )
				newSrc = img.src.replace(/([-@])[12]x\.(gif|jpeg|jpg|png)(\?|$)/, '$1'+newSize+'x.$2$3');
		}

		else {
			return false;
		}

		// Don't set img.src unless it has changed. This avoids unnecessary reloads.
		if ( newSrc != img.src ) {
			// Store the original img.src
			var prevSrc, origSrc = img.getAttribute("src-orig");
			if ( !origSrc ) {
				origSrc = img.src;
				img.setAttribute("src-orig", origSrc);
			}
			// In case of error, revert img.src
			prevSrc = img.src;
			img.onerror = function(){
				img.src = prevSrc;
				if ( img.getAttribute("scale-fail") < scale )
					img.setAttribute("scale-fail", scale);
				img.onerror = null;
			};
			// Finally load the new image
			img.src = newSrc;
		}

		return true;
	}
};

wpcom_img_zoomer.init();
;
/**
 * author Christopher Blum
 *    - based on the idea of Remy Sharp, http://remysharp.com/2009/01/26/element-in-view-event-plugin/
 *    - forked from http://github.com/zuk/jquery.inview/
 */
(function ($) {
  var inviewObjects = {}, viewportSize, viewportOffset,
      d = document, w = window, documentElement = d.documentElement, expando = $.expando;

  $.event.special.inview = {
    add: function(data) {
      inviewObjects[data.guid + "-" + this[expando]] = { data: data, $element: $(this) };
    },

    remove: function(data) {
      try { delete inviewObjects[data.guid + "-" + this[expando]]; } catch(e) {}
    }
  };

  function getViewportSize() {
    var mode, domObject, size = { height: w.innerHeight, width: w.innerWidth };

    // if this is correct then return it. iPad has compat Mode, so will
    // go into check clientHeight/clientWidth (which has the wrong value).
    if (!size.height) {
      mode = d.compatMode;
      if (mode || !$.support.boxModel) { // IE, Gecko
        domObject = mode === 'CSS1Compat' ?
          documentElement : // Standards
          d.body; // Quirks
        size = {
          height: domObject.clientHeight,
          width:  domObject.clientWidth
        };
      }
    }

    return size;
  }

  function getViewportOffset() {
    return {
      top:  w.pageYOffset || documentElement.scrollTop   || d.body.scrollTop,
      left: w.pageXOffset || documentElement.scrollLeft  || d.body.scrollLeft
    };
  }

  function checkInView() {
    var $elements = $(), elementsLength, i = 0;

    $.each(inviewObjects, function(i, inviewObject) {
      var selector  = inviewObject.data.selector,
          $element  = inviewObject.$element;
      $elements = $elements.add(selector ? $element.find(selector) : $element);
    });

    elementsLength = $elements.length;
    if (elementsLength) {
      viewportSize   = viewportSize   || getViewportSize();
      viewportOffset = viewportOffset || getViewportOffset();

      for (; i<elementsLength; i++) {
        // Ignore elements that are not in the DOM tree
        if (!$.contains(documentElement, $elements[i])) {
          continue;
        }

        var element       = $elements[i],
            $element      = $(element),
            elementSize   = {},
            elementOffset = {},
            inView        = $element.data('inview'),
            visiblePartX,
            visiblePartY,
            visiblePartsMerged;

        // for the case where 'display:none' is used in place of 'visibility:hidden'
        // count and sum the above items to get and move closer to the correct values
        // IMPORTANT :: insert element into container empty
        if($element.css('display') == 'none')
        {
            var parentElement = $element.parent();

            elementOffset.top = parentElement.offset().top;
            elementOffset.left = parentElement.offset().left;
            elementSize.height = parentElement.height();
            elementSize.width = parentElement.width();
        } else {
       	    elementSize = { height: $element.height(), width: $element.width() }
       	    elementOffset = $element.offset();
       	}

        // Don't ask me why because I haven't figured out yet:
        // viewportOffset and viewportSize are sometimes suddenly null in Firefox 5.
        // Even though it sounds weird:
        // It seems that the execution of this function is interferred by the onresize/onscroll event
        // where viewportOffset and viewportSize are unset
        if (!viewportOffset || !viewportSize) {
          return;
        }

        if (element.offsetWidth >= 0 && element.offsetHeight >= 0 && element.style.display != "none" &&
            elementOffset.top + elementSize.height > viewportOffset.top &&
            elementOffset.top < viewportOffset.top + viewportSize.height &&
            elementOffset.left + elementSize.width > viewportOffset.left &&
            elementOffset.left < viewportOffset.left + viewportSize.width) {
          visiblePartX = (viewportOffset.left > elementOffset.left ?
            'right' : (viewportOffset.left + viewportSize.width) < (elementOffset.left + elementSize.width) ?
            'left' : 'both');
          visiblePartY = (viewportOffset.top > elementOffset.top ?
            'bottom' : (viewportOffset.top + viewportSize.height) < (elementOffset.top + elementSize.height) ?
            'top' : 'both');
          visiblePartsMerged = visiblePartX + "-" + visiblePartY;
          if (!inView || inView !== visiblePartsMerged) {
            $element.data('inview', visiblePartsMerged).trigger('inview', [true, visiblePartX, visiblePartY]);
          }
        } else if (inView) {
          $element.data('inview', false).trigger('inview', [false]);
        }
      }
    }
  }

  $(w).bind("scroll resize", function() {
    viewportSize = viewportOffset = null;
  });

  // IE < 9 scrolls to focused elements without firing the "scroll" event
  if (!documentElement.addEventListener && documentElement.attachEvent) {
    documentElement.attachEvent("onfocusin", function() {
      viewportOffset = null;
    });
  }

  // Use setInterval in order to also make sure this captures elements within
  // "overflow:scroll" elements or elements that appeared in the dom tree due to
  // dom manipulation and reflow
  // old: $(window).scroll(checkInView);
  //
  // By the way, iOS (iPad, iPhone, ...) seems to not execute, or at least delays
  // intervals while the user scrolls. Therefore the inview event might fire a bit late there
  setInterval(checkInView, 250);
})(jQuery);;
/* global Jetpack, JSON */
/**
 * Resizeable Iframes.
 *
 * Start listening to resize postMessage events for selected iframes:
 * $( selector ).Jetpack( 'resizeable' );
 * - OR -
 * Jetpack.resizeable( 'on', context );
 *
 * Resize selected iframes:
 * $( selector ).Jetpack( 'resizeable', 'resize', { width: 100, height: 200 } );
 * - OR -
 * Jetpack.resizeable( 'resize', { width: 100, height: 200 }, context );
 *
 * Stop listening to resize postMessage events for selected iframes:
 * $( selector ).Jetpack( 'resizeable', 'off' );
 * - OR -
 * Jetpack.resizeable( 'off', context );
 *
 * Stop listening to all resize postMessage events:
 * Jetpack.resizeable( 'off' );
 */
( function ( $ ) {
	var listening = false, // Are we listening for resize postMessage events
		sourceOrigins = [], // What origins are allowed to send resize postMessage events
		$sources = false, // What iframe elements are we tracking resize postMessage events from
		URLtoOrigin, // Utility to convert URLs into origins
		setupListener, // Binds global resize postMessage event handler
		destroyListener, // Unbinds global resize postMessage event handler
		methods; // Jetpack.resizeable methods

	// Setup the Jetpack global
	if ( 'undefined' === typeof window.Jetpack ) {
		window.Jetpack = {
			/**
			 * Handles the two different calling methods:
			 * $( selector ).Jetpack( 'namespace', 'method', context ) // here, context is optional and is used to filter the collection
			 * - vs. -
			 * Jetpack.namespace( 'method', context ) // here context defines the collection
			 *
			 * @internal
			 *
			 * Call as: Jetpack.getTarget.call( this, context )
			 *
			 * @param string context: jQuery selector
			 * @return jQuery|undefined object on which to perform operations or undefined when context cannot be determined
			 */
			getTarget: function ( context ) {
				if ( this instanceof jQuery ) {
					return context ? this.filter( context ) : this;
				}

				return context ? $( context ) : context;
			},
		};
	}

	// Setup the Jetpack jQuery method
	if ( 'undefined' === typeof $.fn.Jetpack ) {
		/**
		 * Dispatches calls to the correct namespace
		 *
		 * @param string namespace
		 * @param ...
		 * @return mixed|jQuery (chainable)
		 */
		$.fn.Jetpack = function ( namespace ) {
			if ( 'function' === typeof Jetpack[ namespace ] ) {
				// Send the call to the correct Jetpack.namespace
				return Jetpack[ namespace ].apply( this, Array.prototype.slice.call( arguments, 1 ) );
			} else {
				$.error( 'Namespace "' + namespace + '" does not exist on jQuery.Jetpack' );
			}
		};
	}

	// Define Jetpack.resizeable() namespace to just always bail if no postMessage
	if ( 'function' !== typeof window.postMessage ) {
		$.extend( window.Jetpack, {
			/**
			 * Defines the Jetpack.resizeable() namespace.
			 * See below for non-trivial definition for browsers with postMessage.
			 */
			resizeable: function () {
				$.error( 'Browser does not support window.postMessage' );
			},
		} );

		return;
	}

	/**
	 * Utility to convert URLs into origins
	 *
	 * http://example.com:port/path?query#fragment -> http://example.com:port
	 *
	 * @param string URL
	 * @return string origin
	 */
	URLtoOrigin = function ( URL ) {
		if ( ! URL.match( /^https?:\/\// ) ) {
			URL = document.location.href;
		}
		return URL.split( '/' ).slice( 0, 3 ).join( '/' );
	};

	/**
	 * Binds global resize postMessage event handler
	 */
	setupListener = function () {
		listening = true;

		$( window ).on( 'message.JetpackResizeableIframe', function ( e ) {
			var event = e.originalEvent,
				data;

			// Ensure origin is allowed
			if ( -1 === $.inArray( event.origin, sourceOrigins ) ) {
				return;
			}

			// Some browsers send structured data, some send JSON strings
			if ( 'object' === typeof event.data ) {
				data = event.data.data;
			} else {
				try {
					data = JSON.parse( event.data );
				} catch ( err ) {
					data = false;
				}
			}

			if ( ! data.data ) {
				return;
			}

			// Un-nest
			data = data.data;

			// Is it a resize event?
			if ( 'undefined' === typeof data.action || 'resize' !== data.action ) {
				return;
			}

			// Find the correct iframe and resize it
			$sources
				.filter( function () {
					if ( 'undefined' !== typeof data.name ) {
						return this.name === data.name;
					} else {
						return event.source === this.contentWindow;
					}
				} )
				.first()
				.Jetpack( 'resizeable', 'resize', data );
		} );
	};

	/**
	 * Unbinds global resize postMessage event handler
	 */
	destroyListener = function () {
		listening = false;
		$( window ).off( 'message.JetpackResizeableIframe' );

		sourceOrigins = [];
		$( '.jetpack-resizeable' ).removeClass( 'jetpack-resizeable' );
		$sources = false;
	};

	// Methods for Jetpack.resizeable() namespace
	methods = {
		/**
		 * Start listening for resize postMessage events on the given iframes
		 *
		 * Call statically as: Jetpack.resizeable( 'on', context )
		 * Call as: $( selector ).Jetpack( 'resizeable', 'on', context ) // context optional: used to filter the collectino
		 *
		 * @param string context jQuery selector.
		 * @return jQuery (chainable)
		 */
		on: function ( context ) {
			var target = Jetpack.getTarget.call( this, context );

			if ( ! listening ) {
				setupListener();
			}

			target
				.each( function () {
					sourceOrigins.push( URLtoOrigin( $( this ).attr( 'src' ) ) );
				} )
				.addClass( 'jetpack-resizeable' );

			$sources = $( '.jetpack-resizeable' );

			return target;
		},

		/**
		 * Stop listening for resize postMessage events on the given iframes
		 *
		 * Call statically as: Jetpack.resizeable( 'off', context )
		 * Call as: $( selector ).Jetpack( 'resizeable', 'off', context ) // context optional: used to filter the collectino
		 *
		 * @param string context jQuery selector
		 * @return jQuery (chainable)
		 */
		off: function ( context ) {
			var target = Jetpack.getTarget.call( this, context );

			if ( 'undefined' === typeof target ) {
				destroyListener();

				return target;
			}

			target
				.each( function () {
					var origin = URLtoOrigin( $( this ).attr( 'src' ) ),
						pos = $.inArray( origin, sourceOrigins );

					if ( -1 !== pos ) {
						sourceOrigins.splice( pos, 1 );
					}
				} )
				.removeClass( 'jetpack-resizeable' );

			$sources = $( '.jetpack-resizeable' );

			return target;
		},

		/**
		 * Resize the given iframes
		 *
		 * Call statically as: Jetpack.resizeable( 'resize', dimensions, context )
		 * Call as: $( selector ).Jetpack( 'resizeable', 'resize', dimensions, context ) // context optional: used to filter the collectino
		 *
		 * @param object dimensions in pixels: { width: (int), height: (int) }
		 * @param string context jQuery selector
		 * @return jQuery (chainable)
		 */
		resize: function ( dimensions, context ) {
			var target = Jetpack.getTarget.call( this, context );

			$.each( [ 'width', 'height' ], function ( i, variable ) {
				var value = 0,
					container;
				if ( 'undefined' !== typeof dimensions[ variable ] ) {
					value = parseInt( dimensions[ variable ], 10 );
				}

				if ( 0 !== value ) {
					target[ variable ]( value );
					container = target.parent();
					if ( container.hasClass( 'slim-likes-widget' ) ) {
						container[ variable ]( value );
					}
				}
			} );

			return target;
		},
	};

	// Define Jetpack.resizeable() namespace
	$.extend( window.Jetpack, {
		/**
		 * Defines the Jetpack.resizeable() namespace.
		 * See above for trivial definition for browsers with no postMessage.
		 *
		 * @param string method
		 * @param ...
		 * @return mixed|jQuery (chainable)
		 */
		resizeable: function ( method ) {
			if ( methods[ method ] ) {
				// Send the call to the correct Jetpack.resizeable() method
				return methods[ method ].apply( this, Array.prototype.slice.call( arguments, 1 ) );
			} else if ( ! method ) {
				// By default, send to Jetpack.resizeable( 'on' ), which isn't useful in that form but is when called as
				// jQuery( selector ).Jetpack( 'resizeable' )
				return methods.on.apply( this );
			} else {
				$.error( 'Method ' + method + ' does not exist on Jetpack.resizeable' );
			}
		},
	} );
} )( jQuery );
;
/* global pm, wpcom_reblog */

var jetpackLikesWidgetQueue = [];
var jetpackLikesWidgetBatch = [];
var jetpackLikesMasterReady = false;

function JetpackLikespostMessage( message, target ) {
	if ( 'string' === typeof message ){
		try {
			message = JSON.parse( message );
		} catch(e) {
			return;
		}
	}

	pm( {
		target: target,
		type: 'likesMessage',
		data: message,
		origin: '*'
	} );
}

function JetpackLikesBatchHandler() {
	var requests = [];
	jQuery( 'div.jetpack-likes-widget-unloaded' ).each( function() {
		if ( jetpackLikesWidgetBatch.indexOf( this.id ) > -1 ) {
			return;
		}
		jetpackLikesWidgetBatch.push( this.id );
		var regex = /like-(post|comment)-wrapper-(\d+)-(\d+)-(\w+)/,
			match = regex.exec( this.id ),
			info;

		if ( ! match || match.length !== 5 ) {
			return;
		}

		info = {
			blog_id: match[2],
			width:   this.width
		};

		if ( 'post' === match[1] ) {
			info.post_id = match[3];
		} else if ( 'comment' === match[1] ) {
			info.comment_id = match[3];
		}

		info.obj_id = match[4];

		requests.push( info );
	});

	if ( requests.length > 0 ) {
		JetpackLikespostMessage( { event: 'initialBatch', requests: requests }, window.frames['likes-master'] );
	}
}

function JetpackLikesMessageListener( event, message ) {
	var allowedOrigin, $container, $list, offset, rowLength, height, scrollbarWidth;

	if ( 'undefined' === typeof event.event ) {
		return;
	}

	// We only allow messages from one origin
	allowedOrigin = window.location.protocol + '//widgets.wp.com';
	if ( allowedOrigin !== message.origin ) {
		return;
	}

	if ( 'masterReady' === event.event ) {
		jQuery( document ).ready( function() {
			jetpackLikesMasterReady = true;

			var stylesData = {
					event: 'injectStyles'
				},
				$sdTextColor = jQuery( '.sd-text-color' ),
				$sdLinkColor = jQuery( '.sd-link-color' );

			if ( jQuery( 'iframe.admin-bar-likes-widget' ).length > 0 ) {
				JetpackLikespostMessage( { event: 'adminBarEnabled' }, window.frames[ 'likes-master' ] );

				stylesData.adminBarStyles = {
					background: jQuery( '#wpadminbar .quicklinks li#wp-admin-bar-wpl-like > a' ).css( 'background' ),
					isRtl: ( 'rtl' === jQuery( '#wpadminbar' ).css( 'direction' ) )
				};
			}

			// enable reblogs if we're on a single post page
			if ( jQuery( 'body' ).hasClass( 'single' ) ) {
				JetpackLikespostMessage( { event: 'reblogsEnabled' }, window.frames[ 'likes-master' ] );
			}

			if ( ! window.addEventListener ) {
				jQuery( '#wp-admin-bar-admin-bar-likes-widget' ).hide();
			}

			stylesData.textStyles = {
				color:          $sdTextColor.css( 'color' ),
				fontFamily:     $sdTextColor.css( 'font-family' ),
				fontSize:       $sdTextColor.css( 'font-size' ),
				direction:      $sdTextColor.css( 'direction' ),
				fontWeight:     $sdTextColor.css( 'font-weight' ),
				fontStyle:      $sdTextColor.css( 'font-style' ),
				textDecoration: $sdTextColor.css('text-decoration')
			};

			stylesData.linkStyles = {
				color:          $sdLinkColor.css('color'),
				fontFamily:     $sdLinkColor.css('font-family'),
				fontSize:       $sdLinkColor.css('font-size'),
				textDecoration: $sdLinkColor.css('text-decoration'),
				fontWeight:     $sdLinkColor.css( 'font-weight' ),
				fontStyle:      $sdLinkColor.css( 'font-style' )
			};

			JetpackLikespostMessage( stylesData, window.frames[ 'likes-master' ] );

			JetpackLikesBatchHandler();

			jQuery( document ).on( 'inview', 'div.jetpack-likes-widget-unloaded', function() {
				jetpackLikesWidgetQueue.push( this.id );
			});
		});
	}

	if ( 'showLikeWidget' === event.event ) {
		jQuery( '#' + event.id + ' .post-likes-widget-placeholder'  ).fadeOut( 'fast', function() {
			jQuery( '#' + event.id + ' .post-likes-widget' ).fadeIn( 'fast', function() {
				JetpackLikespostMessage( { event: 'likeWidgetDisplayed', blog_id: event.blog_id, post_id: event.post_id, obj_id: event.obj_id }, window.frames['likes-master'] );
			});
		});
	}

	if ( 'clickReblogFlair' === event.event ) {
		wpcom_reblog.toggle_reblog_box_flair( event.obj_id );
	}

	if ( 'showOtherGravatars' === event.event ) {
		$container = jQuery( '#likes-other-gravatars' );
		$list = $container.find( 'ul' );

		$container.hide();
		$list.html( '' );

		$container.find( '.likes-text span' ).text( event.total );

		jQuery.each( event.likers, function( i, liker ) {
			var element = jQuery( '<li><a><img /></a></li>' );
			element.addClass( liker.css_class );

			element.find( 'a' ).
				attr({
					href: liker.profile_URL,
					rel: 'nofollow',
					target: '_parent'
				}).
				addClass( 'wpl-liker' );

			element.find( 'img' ).
				attr({
					src: liker.avatar_URL,
					alt: liker.name
				}).
				css({
					width: '30px',
					height: '30px',
					paddingRight: '3px'
				});

			$list.append( element );
		} );

		offset = jQuery( '[name=\'' + event.parent + '\']' ).offset();

		$container.css( 'left', offset.left + event.position.left - 10 + 'px' );
		$container.css( 'top', offset.top + event.position.top - 33 + 'px' );

		rowLength = Math.floor( event.width / 37 );
		height = ( Math.ceil( event.likers.length / rowLength ) * 37 ) + 13;
		if ( height > 204 ) {
			height = 204;
		}

		$container.css( 'height', height + 'px' );
		$container.css( 'width', rowLength * 37 - 7 + 'px' );

		$list.css( 'width', rowLength * 37 + 'px' );

		$container.fadeIn( 'slow' );

		scrollbarWidth = $list[0].offsetWidth - $list[0].clientWidth;
		if ( scrollbarWidth > 0 ) {
			$container.width( $container.width() + scrollbarWidth );
			$list.width( $list.width() + scrollbarWidth );
		}
	}
}

pm.bind( 'likesMessage', JetpackLikesMessageListener );

jQuery( document ).click( function( e ) {
	var $container = jQuery( '#likes-other-gravatars' );

	if ( $container.has( e.target ).length === 0 ) {
		$container.fadeOut( 'slow' );
	}
});

function JetpackLikesWidgetQueueHandler() {
	var $wrapper, wrapperID, found;
	if ( ! jetpackLikesMasterReady ) {
		setTimeout( JetpackLikesWidgetQueueHandler, 500 );
		return;
	}

	if ( jetpackLikesWidgetQueue.length > 0 ) {
		// We may have a widget that needs creating now
		found = false;
		while( jetpackLikesWidgetQueue.length > 0 ) {
			// Grab the first member of the queue that isn't already loading.
			wrapperID = jetpackLikesWidgetQueue.splice( 0, 1 )[0];
			if ( jQuery( '#' + wrapperID ).hasClass( 'jetpack-likes-widget-unloaded' ) ) {
				found = true;
				break;
			}
		}
		if ( ! found ) {
			setTimeout( JetpackLikesWidgetQueueHandler, 500 );
			return;
		}
	} else if ( jQuery( 'div.jetpack-likes-widget-unloaded' ).length > 0 ) {
		// Grab any unloaded widgets for a batch request
		JetpackLikesBatchHandler();

		// Get the next unloaded widget
		wrapperID = jQuery( 'div.jetpack-likes-widget-unloaded' ).first()[0].id;
		if ( ! wrapperID ) {
			// Everything is currently loaded
			setTimeout( JetpackLikesWidgetQueueHandler, 500 );
			return;
		}
	}

	if ( 'undefined' === typeof wrapperID ) {
		setTimeout( JetpackLikesWidgetQueueHandler, 500 );
		return;
	}

	$wrapper = jQuery( '#' + wrapperID );
	$wrapper.find( 'iframe' ).remove();

	if ( $wrapper.hasClass( 'slim-likes-widget' ) ) {
		$wrapper.find( '.post-likes-widget-placeholder' ).after( '<iframe class="post-likes-widget jetpack-likes-widget" name="' + $wrapper.data( 'name' ) + '" height="22px" width="68px" frameBorder="0" scrolling="no" src="' + $wrapper.data( 'src' ) + '"></iframe>' );
	} else {
		$wrapper.find( '.post-likes-widget-placeholder' ).after( '<iframe class="post-likes-widget jetpack-likes-widget" name="' + $wrapper.data( 'name' ) + '" height="55px" width="100%" frameBorder="0" src="' + $wrapper.data( 'src' ) + '"></iframe>' );
	}

	$wrapper.removeClass( 'jetpack-likes-widget-unloaded' ).addClass( 'jetpack-likes-widget-loading' );

	$wrapper.find( 'iframe' ).load( function( e ) {
		var $iframe = jQuery( e.target );
		$wrapper.removeClass( 'jetpack-likes-widget-loading' ).addClass( 'jetpack-likes-widget-loaded' );

		JetpackLikespostMessage( { event: 'loadLikeWidget', name: $iframe.attr( 'name' ), width: $iframe.width(), domain: window.location.hostname }, window.frames[ 'likes-master' ] );

		if ( $wrapper.hasClass( 'slim-likes-widget' ) ) {
			$wrapper.find( 'iframe' ).Jetpack( 'resizeable' );
		}
	});
	setTimeout( JetpackLikesWidgetQueueHandler, 250 );
}
JetpackLikesWidgetQueueHandler();
;
( function( $ ) {
	var cookieValue = document.cookie.replace( /(?:(?:^|.*;\s*)eucookielaw\s*\=\s*([^;]*).*$)|^.*$/, '$1' ),
		overlay = $( '#eu-cookie-law' ),
		container = $( '.widget_eu_cookie_law_widget' ),
		initialScrollPosition,
		scrollFunction;

	if ( overlay.hasClass( 'ads-active' ) ) {
		var adsCookieValue = document.cookie.replace( /(?:(?:^|.*;\s*)personalized-ads-consent\s*\=\s*([^;]*).*$)|^.*$/, '$1' );
		if ( '' !== cookieValue && '' !== adsCookieValue ) {
			overlay.remove();
		}
	} else if ( '' !== cookieValue ) {
		overlay.remove();
	}

	$( '.widget_eu_cookie_law_widget' ).appendTo( 'body' ).fadeIn();

	overlay.find( 'form' ).on( 'submit', accept );

	if ( overlay.hasClass( 'hide-on-scroll' ) ) {
		initialScrollPosition = $( window ).scrollTop();
		scrollFunction = function() {
			if ( Math.abs( $( window ).scrollTop() - initialScrollPosition ) > 50 ) {
				accept();
			}
		};
		$( window ).on( 'scroll', scrollFunction );
	} else if ( overlay.hasClass( 'hide-on-time' ) ) {
		setTimeout( accept, overlay.data( 'hide-timeout' ) * 1000 );
	}

	var accepted = false;
	function accept( event ) {
		if ( accepted ) {
			return;
		}
		accepted = true;

		if ( event && event.preventDefault ) {
			event.preventDefault();
		}

		if ( overlay.hasClass( 'hide-on-scroll' ) ) {
			$( window ).off( 'scroll', scrollFunction );
		}

		var expireTime = new Date();
		expireTime.setTime( expireTime.getTime() + ( overlay.data( 'consent-expiration' ) * 24 * 60 * 60 * 1000 ) );

		document.cookie = 'eucookielaw=' + expireTime.getTime() + ';path=/;expires=' + expireTime.toGMTString();
		if ( overlay.hasClass( 'ads-active' ) && overlay.hasClass( 'hide-on-button' ) ) {
			document.cookie = 'personalized-ads-consent=' + expireTime.getTime() + ';path=/;expires=' + expireTime.toGMTString();
		}

		overlay.fadeOut( 400, function() {
			overlay.remove();
			container.remove();
		} );
	}
} )( jQuery );
;
!function(){"use strict";var e,n,t={843:function(e,n,t){t.d(n,{pM:function(){return o},qq:function(){return r},zq:function(){return i},a3:function(){return l},hA:function(){return u},d6:function(){return c},Uq:function(){return a},j4:function(){return s},B$:function(){return _}});var o={BANNER:"banner",MODAL:"modal",HIDDEN:""},r={PURPOSE:"purpose",VENDOR:"vendor"},i=258,l={1:!0,2:!0,3:!0,4:!0,7:!0,9:!0,10:!0},u={2:!0,3:!0,4:!0,7:!0,9:!0,10:!0},c={UI_SHOWN:"cmpuishown",LOADED:"tcloaded",USER_ACTION_COMPLETE:"useractioncomplete"},a={VISIBLE:"visible",HIDDEN:"hidden",DISABLED:"disabled"},s={STUB:"stub",LOADED:"loaded",ERROR:"error"},_="euconsent-v2"},748:function(e,n,t){t.d(n,{n4:function(){return S},Vo:function(){return C}});var o,r,i=t(400),l=[],u=i.YM.__b,c=i.YM.__r,a=i.YM.diffed,s=i.YM.__c,_=i.YM.unmount;function f(){l.forEach((function(e){if(e.__P)try{e.__H.__h.forEach(p),e.__H.__h.forEach(h),e.__H.__h=[]}catch(n){e.__H.__h=[],i.YM.__e(n,e.__v)}})),l=[]}i.YM.__b=function(e){o=null,u&&u(e)},i.YM.__r=function(e){c&&c(e),0;var n=(o=e.__c).__H;n&&(n.__h.forEach(p),n.__h.forEach(h),n.__h=[])},i.YM.diffed=function(e){a&&a(e);var n=e.__c;n&&n.__H&&n.__H.__h.length&&(1!==l.push(n)&&r===i.YM.requestAnimationFrame||((r=i.YM.requestAnimationFrame)||function(e){var n,t=function(){clearTimeout(o),d&&cancelAnimationFrame(n),setTimeout(e)},o=setTimeout(t,100);d&&(n=requestAnimationFrame(t))})(f)),o=void 0},i.YM.__c=function(e,n){n.some((function(e){try{e.__h.forEach(p),e.__h=e.__h.filter((function(e){return!e.__||h(e)}))}catch(t){n.some((function(e){e.__h&&(e.__h=[])})),n=[],i.YM.__e(t,e.__v)}})),s&&s(e,n)},i.YM.unmount=function(e){_&&_(e);var n=e.__c;if(n&&n.__H)try{n.__H.__.forEach(p)}catch(e){i.YM.__e(e,n.__v)}};var d="function"==typeof requestAnimationFrame;function p(e){var n=o;"function"==typeof e.__c&&e.__c(),o=n}function h(e){var n=o;e.__c=e.__(),o=n}function v(e,n){for(var t in n)e[t]=n[t];return e}function y(e,n){for(var t in e)if("__source"!==t&&!(t in n))return!0;for(var o in n)if("__source"!==o&&e[o]!==n[o])return!0;return!1}function m(e){this.props=e}(m.prototype=new i.wA).isPureReactComponent=!0,m.prototype.shouldComponentUpdate=function(e,n){return y(this.props,e)||y(this.state,n)};var b=i.YM.__b;i.YM.__b=function(e){e.type&&e.type.__f&&e.ref&&(e.props.ref=e.ref,e.ref=null),b&&b(e)};"undefined"!=typeof Symbol&&Symbol.for&&Symbol.for("react.forward_ref");var g=function(e,n){return null==e?null:(0,i.bR)((0,i.bR)(e).map(n))},k=(i.bR,i.YM.__e);i.YM.__e=function(e,n,t){if(e.then)for(var o,r=n;r=r.__;)if((o=r.__c)&&o.__c)return null==n.__e&&(n.__e=t.__e,n.__k=t.__k),o.__c(e,n);k(e,n,t)};var w=i.YM.unmount;function S(){this.__u=0,this.t=null,this.__b=null}function E(e){var n=e.__.__c;return n&&n.__e&&n.__e(e)}function C(e){var n,t,o;function r(r){if(n||(n=e()).then((function(e){t=e.default||e}),(function(e){o=e})),o)throw o;if(!t)throw n;return(0,i.az)(t,r)}return r.displayName="Lazy",r.__f=!0,r}function L(){this.u=null,this.o=null}i.YM.unmount=function(e){var n=e.__c;n&&n.__R&&n.__R(),n&&!0===e.__h&&(e.type=null),w&&w(e)},(S.prototype=new i.wA).__c=function(e,n){var t=n.__c,o=this;null==o.t&&(o.t=[]),o.t.push(t);var r=E(o.__v),i=!1,l=function(){i||(i=!0,t.__R=null,r?r(u):u())};t.__R=l;var u=function(){if(!--o.__u){if(o.state.__e){var e=o.state.__e;o.__v.__k[0]=function e(n,t,o){return n&&(n.__v=null,n.__k=n.__k&&n.__k.map((function(n){return e(n,t,o)})),n.__c&&n.__c.__P===t&&(n.__e&&o.insertBefore(n.__e,n.__d),n.__c.__e=!0,n.__c.__P=o)),n}(e,e.__c.__P,e.__c.__O)}var n;for(o.setState({__e:o.__b=null});n=o.t.pop();)n.forceUpdate()}},c=!0===n.__h;o.__u++||c||o.setState({__e:o.__b=o.__v.__k[0]}),e.then(l,l)},S.prototype.componentWillUnmount=function(){this.t=[]},S.prototype.render=function(e,n){if(this.__b){if(this.__v.__k){var t=document.createElement("div"),o=this.__v.__k[0].__c;this.__v.__k[0]=function e(n,t,o){return n&&(n.__c&&n.__c.__H&&(n.__c.__H.__.forEach((function(e){"function"==typeof e.__c&&e.__c()})),n.__c.__H=null),null!=(n=v({},n)).__c&&(n.__c.__P===o&&(n.__c.__P=t),n.__c=null),n.__k=n.__k&&n.__k.map((function(n){return e(n,t,o)}))),n}(this.__b,t,o.__O=o.__P)}this.__b=null}var r=n.__e&&(0,i.az)(i.HY,null,e.fallback);return r&&(r.__h=null),[(0,i.az)(i.HY,null,n.__e?null:e.children),r]};var A=function(e,n,t){if(++t[1]===t[0]&&e.o.delete(n),e.props.revealOrder&&("t"!==e.props.revealOrder[0]||!e.o.size))for(t=e.u;t;){for(;t.length>3;)t.pop()();if(t[1]<t[0])break;e.u=t=t[2]}};(L.prototype=new i.wA).__e=function(e){var n=this,t=E(n.__v),o=n.o.get(e);return o[0]++,function(r){var i=function(){n.props.revealOrder?(o.push(r),A(n,e,o)):r()};t?t(i):i()}},L.prototype.render=function(e){this.u=null,this.o=new Map;var n=(0,i.bR)(e.children);e.revealOrder&&"b"===e.revealOrder[0]&&n.reverse();for(var t=n.length;t--;)this.o.set(n[t],this.u=[1,0,this.u]);return e.children},L.prototype.componentDidUpdate=L.prototype.componentDidMount=function(){var e=this;this.o.forEach((function(n,t){A(e,t,n)}))};var P="undefined"!=typeof Symbol&&Symbol.for&&Symbol.for("react.element")||60103,O=/^(?:accent|alignment|arabic|baseline|cap|clip(?!PathU)|color|fill|flood|font|glyph(?!R)|horiz|marker(?!H|W|U)|overline|paint|stop|strikethrough|stroke|text(?!L)|underline|unicode|units|v|vector|vert|word|writing|x(?!C))[A-Z]/,M=function(e){return("undefined"!=typeof Symbol&&"symbol"==typeof Symbol()?/fil|che|rad/i:/fil|che|ra/i).test(e)};i.wA.prototype.isReactComponent={},["componentWillMount","componentWillReceiveProps","componentWillUpdate"].forEach((function(e){Object.defineProperty(i.wA.prototype,e,{configurable:!0,get:function(){return this["UNSAFE_"+e]},set:function(n){Object.defineProperty(this,e,{configurable:!0,writable:!0,value:n})}})}));var I=i.YM.event;function T(){}function H(){return this.cancelBubble}function U(){return this.defaultPrevented}i.YM.event=function(e){return I&&(e=I(e)),e.persist=T,e.isPropagationStopped=H,e.isDefaultPrevented=U,e.nativeEvent=e};var D={configurable:!0,get:function(){return this.class}},V=i.YM.vnode;i.YM.vnode=function(e){var n=e.type,t=e.props,o=t;if("string"==typeof n){for(var r in o={},t){var l=t[r];"value"===r&&"defaultValue"in t&&null==l||("defaultValue"===r&&"value"in t&&null==t.value?r="value":"download"===r&&!0===l?l="":/ondoubleclick/i.test(r)?r="ondblclick":/^onchange(textarea|input)/i.test(r+n)&&!M(t.type)?r="oninput":/^on(Ani|Tra|Tou|BeforeInp)/.test(r)?r=r.toLowerCase():O.test(r)?r=r.replace(/[A-Z0-9]/,"-$&").toLowerCase():null===l&&(l=void 0),o[r]=l)}"select"==n&&o.multiple&&Array.isArray(o.value)&&(o.value=(0,i.bR)(t.children).forEach((function(e){e.props.selected=-1!=o.value.indexOf(e.props.value)}))),"select"==n&&null!=o.defaultValue&&(o.value=(0,i.bR)(t.children).forEach((function(e){e.props.selected=o.multiple?-1!=o.defaultValue.indexOf(e.props.value):o.defaultValue==e.props.value}))),e.props=o}n&&t.class!=t.className&&(D.enumerable="className"in t,null!=t.className&&(o.class=t.className),Object.defineProperty(o,"className",D)),e.$$typeof=P,V&&V(e)};var N=i.YM.__r;i.YM.__r=function(e){N&&N(e),e.__c};"object"==typeof performance&&"function"==typeof performance.now&&performance.now.bind(performance);i.HY,i.az,i.kr,i.Vf,i.HY,i.wA,i.HY},400:function(e,n,t){t.d(n,{sY:function(){return V},ZB:function(){return N},az:function(){return d},h:function(){return d},HY:function(){return v},Vf:function(){return h},wA:function(){return y},Tm:function(){return x},kr:function(){return j},bR:function(){return E},YM:function(){return o}});var o,r,i,l,u,c={},a=[],s=/acit|ex(?:s|g|n|p|$)|rph|grid|ows|mnc|ntw|ine[ch]|zoo|^ord|itera/i;function _(e,n){for(var t in n)e[t]=n[t];return e}function f(e){var n=e.parentNode;n&&n.removeChild(e)}function d(e,n,t){var o,r,i,l=arguments,u={};for(i in n)"key"==i?o=n[i]:"ref"==i?r=n[i]:u[i]=n[i];if(arguments.length>3)for(t=[t],i=3;i<arguments.length;i++)t.push(l[i]);if(null!=t&&(u.children=t),"function"==typeof e&&null!=e.defaultProps)for(i in e.defaultProps)void 0===u[i]&&(u[i]=e.defaultProps[i]);return p(e,u,o,r,null)}function p(e,n,t,r,i){var l={type:e,props:n,key:t,ref:r,__k:null,__:null,__b:0,__e:null,__d:void 0,__c:null,__h:null,constructor:void 0,__v:null==i?++o.__v:i};return null!=o.vnode&&o.vnode(l),l}function h(){return{current:null}}function v(e){return e.children}function y(e,n){this.props=e,this.context=n}function m(e,n){if(null==n)return e.__?m(e.__,e.__.__k.indexOf(e)+1):null;for(var t;n<e.__k.length;n++)if(null!=(t=e.__k[n])&&null!=t.__e)return t.__e;return"function"==typeof e.type?m(e):null}function b(e){var n,t;if(null!=(e=e.__)&&null!=e.__c){for(e.__e=e.__c.base=null,n=0;n<e.__k.length;n++)if(null!=(t=e.__k[n])&&null!=t.__e){e.__e=e.__c.base=t.__e;break}return b(e)}}function g(e){(!e.__d&&(e.__d=!0)&&r.push(e)&&!k.__r++||l!==o.debounceRendering)&&((l=o.debounceRendering)||i)(k)}function k(){for(var e;k.__r=r.length;)e=r.sort((function(e,n){return e.__v.__b-n.__v.__b})),r=[],e.some((function(e){var n,t,o,r,i,l;e.__d&&(i=(r=(n=e).__v).__e,(l=n.__P)&&(t=[],(o=_({},r)).__v=r.__v+1,M(l,r,o,n.__n,void 0!==l.ownerSVGElement,null!=r.__h?[i]:null,t,null==i?m(r):i,r.__h),I(t,r),r.__e!=i&&b(r)))}))}function w(e,n,t,o,r,i,l,u,s,_){var f,d,h,y,b,g,k,w=o&&o.__k||a,E=w.length;for(t.__k=[],f=0;f<n.length;f++)if(null!=(y=t.__k[f]=null==(y=n[f])||"boolean"==typeof y?null:"string"==typeof y||"number"==typeof y||"bigint"==typeof y?p(null,y,null,null,y):Array.isArray(y)?p(v,{children:y},null,null,null):y.__b>0?p(y.type,y.props,y.key,null,y.__v):y)){if(y.__=t,y.__b=t.__b+1,null===(h=w[f])||h&&y.key==h.key&&y.type===h.type)w[f]=void 0;else for(d=0;d<E;d++){if((h=w[d])&&y.key==h.key&&y.type===h.type){w[d]=void 0;break}h=null}M(e,y,h=h||c,r,i,l,u,s,_),b=y.__e,(d=y.ref)&&h.ref!=d&&(k||(k=[]),h.ref&&k.push(h.ref,null,y),k.push(d,y.__c||b,y)),null!=b?(null==g&&(g=b),"function"==typeof y.type&&null!=y.__k&&y.__k===h.__k?y.__d=s=S(y,s,e):s=C(e,y,h,w,b,s),_||"option"!==t.type?"function"==typeof t.type&&(t.__d=s):e.value=""):s&&h.__e==s&&s.parentNode!=e&&(s=m(h))}for(t.__e=g,f=E;f--;)null!=w[f]&&("function"==typeof t.type&&null!=w[f].__e&&w[f].__e==t.__d&&(t.__d=m(o,f+1)),U(w[f],w[f]));if(k)for(f=0;f<k.length;f++)H(k[f],k[++f],k[++f])}function S(e,n,t){var o,r;for(o=0;o<e.__k.length;o++)(r=e.__k[o])&&(r.__=e,n="function"==typeof r.type?S(r,n,t):C(t,r,r,e.__k,r.__e,n));return n}function E(e,n){return n=n||[],null==e||"boolean"==typeof e||(Array.isArray(e)?e.some((function(e){E(e,n)})):n.push(e)),n}function C(e,n,t,o,r,i){var l,u,c;if(void 0!==n.__d)l=n.__d,n.__d=void 0;else if(null==t||r!=i||null==r.parentNode)e:if(null==i||i.parentNode!==e)e.appendChild(r),l=null;else{for(u=i,c=0;(u=u.nextSibling)&&c<o.length;c+=2)if(u==r)break e;e.insertBefore(r,i),l=i}return void 0!==l?l:r.nextSibling}function L(e,n,t){"-"===n[0]?e.setProperty(n,t):e[n]=null==t?"":"number"!=typeof t||s.test(n)?t:t+"px"}function A(e,n,t,o,r){var i;e:if("style"===n)if("string"==typeof t)e.style.cssText=t;else{if("string"==typeof o&&(e.style.cssText=o=""),o)for(n in o)t&&n in t||L(e.style,n,"");if(t)for(n in t)o&&t[n]===o[n]||L(e.style,n,t[n])}else if("o"===n[0]&&"n"===n[1])i=n!==(n=n.replace(/Capture$/,"")),n=n.toLowerCase()in e?n.toLowerCase().slice(2):n.slice(2),e.l||(e.l={}),e.l[n+i]=t,t?o||e.addEventListener(n,i?O:P,i):e.removeEventListener(n,i?O:P,i);else if("dangerouslySetInnerHTML"!==n){if(r)n=n.replace(/xlink[H:h]/,"h").replace(/sName$/,"s");else if("href"!==n&&"list"!==n&&"form"!==n&&"tabIndex"!==n&&"download"!==n&&n in e)try{e[n]=null==t?"":t;break e}catch(e){}"function"==typeof t||(null!=t&&(!1!==t||"a"===n[0]&&"r"===n[1])?e.setAttribute(n,t):e.removeAttribute(n))}}function P(e){this.l[e.type+!1](o.event?o.event(e):e)}function O(e){this.l[e.type+!0](o.event?o.event(e):e)}function M(e,n,t,r,i,l,u,c,a){var s,f,d,p,h,m,b,g,k,S,E,C=n.type;if(void 0!==n.constructor)return null;null!=t.__h&&(a=t.__h,c=n.__e=t.__e,n.__h=null,l=[c]),(s=o.__b)&&s(n);try{e:if("function"==typeof C){if(g=n.props,k=(s=C.contextType)&&r[s.__c],S=s?k?k.props.value:s.__:r,t.__c?b=(f=n.__c=t.__c).__=f.__E:("prototype"in C&&C.prototype.render?n.__c=f=new C(g,S):(n.__c=f=new y(g,S),f.constructor=C,f.render=D),k&&k.sub(f),f.props=g,f.state||(f.state={}),f.context=S,f.__n=r,d=f.__d=!0,f.__h=[]),null==f.__s&&(f.__s=f.state),null!=C.getDerivedStateFromProps&&(f.__s==f.state&&(f.__s=_({},f.__s)),_(f.__s,C.getDerivedStateFromProps(g,f.__s))),p=f.props,h=f.state,d)null==C.getDerivedStateFromProps&&null!=f.componentWillMount&&f.componentWillMount(),null!=f.componentDidMount&&f.__h.push(f.componentDidMount);else{if(null==C.getDerivedStateFromProps&&g!==p&&null!=f.componentWillReceiveProps&&f.componentWillReceiveProps(g,S),!f.__e&&null!=f.shouldComponentUpdate&&!1===f.shouldComponentUpdate(g,f.__s,S)||n.__v===t.__v){f.props=g,f.state=f.__s,n.__v!==t.__v&&(f.__d=!1),f.__v=n,n.__e=t.__e,n.__k=t.__k,n.__k.forEach((function(e){e&&(e.__=n)})),f.__h.length&&u.push(f);break e}null!=f.componentWillUpdate&&f.componentWillUpdate(g,f.__s,S),null!=f.componentDidUpdate&&f.__h.push((function(){f.componentDidUpdate(p,h,m)}))}f.context=S,f.props=g,f.state=f.__s,(s=o.__r)&&s(n),f.__d=!1,f.__v=n,f.__P=e,s=f.render(f.props,f.state,f.context),f.state=f.__s,null!=f.getChildContext&&(r=_(_({},r),f.getChildContext())),d||null==f.getSnapshotBeforeUpdate||(m=f.getSnapshotBeforeUpdate(p,h)),E=null!=s&&s.type===v&&null==s.key?s.props.children:s,w(e,Array.isArray(E)?E:[E],n,t,r,i,l,u,c,a),f.base=n.__e,n.__h=null,f.__h.length&&u.push(f),b&&(f.__E=f.__=null),f.__e=!1}else null==l&&n.__v===t.__v?(n.__k=t.__k,n.__e=t.__e):n.__e=T(t.__e,n,t,r,i,l,u,a);(s=o.diffed)&&s(n)}catch(e){n.__v=null,(a||null!=l)&&(n.__e=c,n.__h=!!a,l[l.indexOf(c)]=null),o.__e(e,n,t)}}function I(e,n){o.__c&&o.__c(n,e),e.some((function(n){try{e=n.__h,n.__h=[],e.some((function(e){e.call(n)}))}catch(e){o.__e(e,n.__v)}}))}function T(e,n,t,o,r,i,l,u){var s,_,d,p,h=t.props,v=n.props,y=n.type,m=0;if("svg"===y&&(r=!0),null!=i)for(;m<i.length;m++)if((s=i[m])&&(s===e||(y?s.localName==y:3==s.nodeType))){e=s,i[m]=null;break}if(null==e){if(null===y)return document.createTextNode(v);e=r?document.createElementNS("http://www.w3.org/2000/svg",y):document.createElement(y,v.is&&v),i=null,u=!1}if(null===y)h===v||u&&e.data===v||(e.data=v);else{if(i=i&&a.slice.call(e.childNodes),_=(h=t.props||c).dangerouslySetInnerHTML,d=v.dangerouslySetInnerHTML,!u){if(null!=i)for(h={},p=0;p<e.attributes.length;p++)h[e.attributes[p].name]=e.attributes[p].value;(d||_)&&(d&&(_&&d.__html==_.__html||d.__html===e.innerHTML)||(e.innerHTML=d&&d.__html||""))}if(function(e,n,t,o,r){var i;for(i in t)"children"===i||"key"===i||i in n||A(e,i,null,t[i],o);for(i in n)r&&"function"!=typeof n[i]||"children"===i||"key"===i||"value"===i||"checked"===i||t[i]===n[i]||A(e,i,n[i],t[i],o)}(e,v,h,r,u),d)n.__k=[];else if(m=n.props.children,w(e,Array.isArray(m)?m:[m],n,t,o,r&&"foreignObject"!==y,i,l,e.firstChild,u),null!=i)for(m=i.length;m--;)null!=i[m]&&f(i[m]);u||("value"in v&&void 0!==(m=v.value)&&(m!==e.value||"progress"===y&&!m)&&A(e,"value",m,h.value,!1),"checked"in v&&void 0!==(m=v.checked)&&m!==e.checked&&A(e,"checked",m,h.checked,!1))}return e}function H(e,n,t){try{"function"==typeof e?e(n):e.current=n}catch(e){o.__e(e,t)}}function U(e,n,t){var r,i,l;if(o.unmount&&o.unmount(e),(r=e.ref)&&(r.current&&r.current!==e.__e||H(r,null,n)),t||"function"==typeof e.type||(t=null!=(i=e.__e)),e.__e=e.__d=void 0,null!=(r=e.__c)){if(r.componentWillUnmount)try{r.componentWillUnmount()}catch(e){o.__e(e,n)}r.base=r.__P=null}if(r=e.__k)for(l=0;l<r.length;l++)r[l]&&U(r[l],n,t);null!=i&&f(i)}function D(e,n,t){return this.constructor(e,t)}function V(e,n,t){var r,i,l;o.__&&o.__(e,n),i=(r="function"==typeof t)?null:t&&t.__k||n.__k,l=[],M(n,e=(!r&&t||n).__k=d(v,null,[e]),i||c,c,void 0!==n.ownerSVGElement,!r&&t?[t]:i?null:n.firstChild?a.slice.call(n.childNodes):null,l,!r&&t?t:i?i.__e:n.firstChild,r),I(l,e)}function N(e,n){V(e,n,N)}function x(e,n,t){var o,r,i,l=arguments,u=_({},e.props);for(i in n)"key"==i?o=n[i]:"ref"==i?r=n[i]:u[i]=n[i];if(arguments.length>3)for(t=[t],i=3;i<arguments.length;i++)t.push(l[i]);return null!=t&&(u.children=t),p(e.type,u,o||e.key,r||e.ref,null)}function j(e,n){var t={__c:n="__cC"+u++,__:e,Consumer:function(e,n){return e.children(n)},Provider:function(e){var t,o;return this.getChildContext||(t=[],(o={})[n]=this,this.getChildContext=function(){return o},this.shouldComponentUpdate=function(e){this.props.value!==e.value&&t.some(g)},this.sub=function(e){t.push(e);var n=e.componentWillUnmount;e.componentWillUnmount=function(){t.splice(t.indexOf(e),1),n&&n.call(e)}}),e.children}};return t.Provider.__=t.Consumer.contextType=t}o={__e:function(e,n){for(var t,o,r;n=n.__;)if((t=n.__c)&&!t.__)try{if((o=t.constructor)&&null!=o.getDerivedStateFromError&&(t.setState(o.getDerivedStateFromError(e)),r=t.__d),null!=t.componentDidCatch&&(t.componentDidCatch(e),r=t.__d),r)return t.__E=t}catch(n){e=n}throw e},__v:0},y.prototype.setState=function(e,n){var t;t=null!=this.__s&&this.__s!==this.state?this.__s:this.__s=_({},this.state),"function"==typeof e&&(e=e(_({},t),this.props)),e&&_(t,e),null!=e&&this.__v&&(n&&this.__h.push(n),g(this))},y.prototype.forceUpdate=function(e){this.__v&&(this.__e=!0,e&&this.__h.push(e),g(this))},y.prototype.render=v,r=[],i="function"==typeof Promise?Promise.prototype.then.bind(Promise.resolve()):setTimeout,k.__r=0,u=0}},o={};function r(e){var n=o[e];if(void 0!==n)return n.exports;var i=o[e]={id:e,exports:{}};return t[e](i,i.exports,r),i.exports}r.m=t,r.amdO={},r.n=function(e){var n=e&&e.__esModule?function(){return e.default}:function(){return e};return r.d(n,{a:n}),n},r.d=function(e,n){for(var t in n)r.o(n,t)&&!r.o(e,t)&&Object.defineProperty(e,t,{enumerable:!0,get:n[t]})},r.f={},r.e=function(e){return Promise.all(Object.keys(r.f).reduce((function(n,t){return r.f[t](e,n),n}),[]))},r.u=function(e){return{143:"app",392:"banner",582:"modal"}[e]+".bundle.js?id="+{143:"10f08b851d8a01803359",392:"69626cb5d25b886923fc",582:"14cc5ce046861105f9cb"}[e]},r.o=function(e,n){return Object.prototype.hasOwnProperty.call(e,n)},e={},n="a8c-cmp:",r.l=function(t,o,i,l){if(e[t])e[t].push(o);else{var u,c;if(void 0!==i)for(var a=document.getElementsByTagName("script"),s=0;s<a.length;s++){var _=a[s];if(_.getAttribute("src")==t||_.getAttribute("data-webpack")==n+i){u=_;break}}u||(c=!0,(u=document.createElement("script")).charset="utf-8",u.timeout=120,r.nc&&u.setAttribute("nonce",r.nc),u.setAttribute("data-webpack",n+i),u.src=t),e[t]=[o];var f=function(n,o){u.onerror=u.onload=null,clearTimeout(d);var r=e[t];if(delete e[t],u.parentNode&&u.parentNode.removeChild(u),r&&r.forEach((function(e){return e(o)})),n)return n(o)},d=setTimeout(f.bind(null,void 0,{type:"timeout",target:u}),12e4);u.onerror=f.bind(null,u.onerror),u.onload=f.bind(null,u.onload),c&&document.head.appendChild(u)}},r.r=function(e){"undefined"!=typeof Symbol&&Symbol.toStringTag&&Object.defineProperty(e,Symbol.toStringTag,{value:"Module"}),Object.defineProperty(e,"__esModule",{value:!0})},r.p="/",function(){var e={968:0};r.f.j=function(n,t){var o=r.o(e,n)?e[n]:void 0;if(0!==o)if(o)t.push(o[2]);else{var i=new Promise((function(t,r){o=e[n]=[t,r]}));t.push(o[2]=i);var l=r.p+r.u(n),u=new Error;r.l(l,(function(t){if(r.o(e,n)&&(0!==(o=e[n])&&(e[n]=void 0),o)){var i=t&&("load"===t.type?"missing":t.type),l=t&&t.target&&t.target.src;u.message="Loading chunk "+n+" failed.\n("+i+": "+l+")",u.name="ChunkLoadError",u.type=i,u.request=l,o[1](u)}}),"chunk-"+n,n)}};var n=function(n,t){var o,i,l=t[0],u=t[1],c=t[2],a=0;for(o in u)r.o(u,o)&&(r.m[o]=u[o]);for(c&&c(r),n&&n(t);a<l.length;a++)i=l[a],r.o(e,i)&&e[i]&&e[i][0](),e[l[a]]=0},t=self.webpackChunka8c_cmp=self.webpackChunka8c_cmp||[];t.forEach(n.bind(null,0)),t.push=n.bind(null,t.push.bind(t))}(),function(){var e=r(400),n=r(748);function t(e,n){for(var t=0;t<n.length;t++){var o=n[t];o.enumerable=o.enumerable||!1,o.configurable=!0,"value"in o&&(o.writable=!0),Object.defineProperty(e,o.key,o)}}var o=function(){function e(){!function(e,n){if(!(e instanceof n))throw new TypeError("Cannot call a class as a function")}(this,e)}var n,o,r;return n=e,r=[{key:"decode",value:function(n){for(var t="",o=0;o<n.length;o++){for(var r=e.dictionary.indexOf(n[o]).toString(2),i=6-r.length,l=0;l<i;l++)t+="0";t+=r}return t}},{key:"encode",value:function(n){for(var t="",o=24-n.length%24,r=0;r<o;r++)n+="0";for(var i=0;i<n.length;i+=6)t+=e.dictionary[parseInt(n.substr(i,6),2)];return t}}],(o=null)&&t(n.prototype,o),r&&t(n,r),e}();function i(e,n){for(var t=0;t<n.length;t++){var o=n[t];o.enumerable=o.enumerable||!1,o.configurable=!0,"value"in o&&(o.writable=!0),Object.defineProperty(e,o.key,o)}}o.dictionary="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";var l=function(){function e(){!function(e,n){if(!(e instanceof n))throw new TypeError("Cannot call a class as a function")}(this,e)}var n,t,o;return n=e,o=[{key:"decode",value:function(e){return parseInt(e,2)}},{key:"encode",value:function(e,n){for(var t="",o=e.toString(2),r=n-o.length,i=0;i<r;i++)t+="0";return t+o}}],(t=null)&&i(n.prototype,t),o&&i(n,o),e}();function u(e,n){for(var t=0;t<n.length;t++){var o=n[t];o.enumerable=o.enumerable||!1,o.configurable=!0,"value"in o&&(o.writable=!0),Object.defineProperty(e,o.key,o)}}var c=function(){function e(){!function(e,n){if(!(e instanceof n))throw new TypeError("Cannot call a class as a function")}(this,e)}var n,t,o;return n=e,o=[{key:"decode",value:function(e){return new Date(100*l.decode(e))}},{key:"encode",value:function(e){return l.encode(Math.round(e.getTime()/100),36)}}],(t=null)&&u(n.prototype,t),o&&u(n,o),e}();function a(e,n){for(var t=0;t<n.length;t++){var o=n[t];o.enumerable=o.enumerable||!1,o.configurable=!0,"value"in o&&(o.writable=!0),Object.defineProperty(e,o.key,o)}}var s=function(){function e(){!function(e,n){if(!(e instanceof n))throw new TypeError("Cannot call a class as a function")}(this,e)}var n,t,o;return n=e,o=[{key:"decode",value:function(e){return String.fromCharCode(l.decode(e)+65)}},{key:"encode",value:function(e){return l.encode(e.charCodeAt(0)-65,6)}}],(t=null)&&a(n.prototype,t),o&&a(n,o),e}();function _(e,n){for(var t=0;t<n.length;t++){var o=n[t];o.enumerable=o.enumerable||!1,o.configurable=!0,"value"in o&&(o.writable=!0),Object.defineProperty(e,o.key,o)}}var f=function(){function e(){!function(e,n){if(!(e instanceof n))throw new TypeError("Cannot call a class as a function")}(this,e)}var n,t,o;return n=e,o=[{key:"decode",value:function(e){return[s.decode(e.substring(0,6)),s.decode(e.substring(6))].join("")}},{key:"encode",value:function(e){return s.encode(e[0],6)+s.encode(e[1],6)}}],(t=null)&&_(n.prototype,t),o&&_(n,o),e}();function d(e,n){for(var t=0;t<n.length;t++){var o=n[t];o.enumerable=o.enumerable||!1,o.configurable=!0,"value"in o&&(o.writable=!0),Object.defineProperty(e,o.key,o)}}var p=function(){function e(){!function(e,n){if(!(e instanceof n))throw new TypeError("Cannot call a class as a function")}(this,e)}var n,t,o;return n=e,o=[{key:"decode",value:function(e){return"1"===e}},{key:"encode",value:function(e){return e?"1":"0"}}],(t=null)&&d(n.prototype,t),o&&d(n,o),e}();function h(e,n){for(var t=0;t<n.length;t++){var o=n[t];o.enumerable=o.enumerable||!1,o.configurable=!0,"value"in o&&(o.writable=!0),Object.defineProperty(e,o.key,o)}}var v=function(){function e(){!function(e,n){if(!(e instanceof n))throw new TypeError("Cannot call a class as a function")}(this,e)}var n,t,o;return n=e,o=[{key:"decode",value:function(e){for(var n={},t=0;t<e.length;t++)"1"===e[t]&&(n[t+1]=!0);return n}},{key:"encode",value:function(e,n){for(var t="",o=1;o<=n;o++)t+=e[o]?"1":"0";return t}}],(t=null)&&h(n.prototype,t),o&&h(n,o),e}();function y(e,n){for(var t=0;t<n.length;t++){var o=n[t];o.enumerable=o.enumerable||!1,o.configurable=!0,"value"in o&&(o.writable=!0),Object.defineProperty(e,o.key,o)}}var m=function(){function e(n){!function(e,n){if(!(e instanceof n))throw new TypeError("Cannot call a class as a function")}(this,e),this.bitString=n,this.position=0}var n,t,o;return n=e,(t=[{key:"advance",value:function(e){this.position+=e}},{key:"getPosition",value:function(){return this.position}},{key:"peekToEnd",value:function(){return this.bitString.substring(this.position)}},{key:"popBits",value:function(e){var n=this.bitString.substring(this.position,this.position+e);return this.advance(e),n}}])&&y(n.prototype,t),o&&y(n,o),e}();function b(e){return function(e){if(Array.isArray(e))return g(e)}(e)||function(e){if("undefined"!=typeof Symbol&&Symbol.iterator in Object(e))return Array.from(e)}(e)||function(e,n){if(!e)return;if("string"==typeof e)return g(e,n);var t=Object.prototype.toString.call(e).slice(8,-1);"Object"===t&&e.constructor&&(t=e.constructor.name);if("Map"===t||"Set"===t)return Array.from(e);if("Arguments"===t||/^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(t))return g(e,n)}(e)||function(){throw new TypeError("Invalid attempt to spread non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.")}()}function g(e,n){(null==n||n>e.length)&&(n=e.length);for(var t=0,o=new Array(n);t<n;t++)o[t]=e[t];return o}function k(e,n){for(var t=0;t<n.length;t++){var o=n[t];o.enumerable=o.enumerable||!1,o.configurable=!0,"value"in o&&(o.writable=!0),Object.defineProperty(e,o.key,o)}}var w=function(){function e(){!function(e,n){if(!(e instanceof n))throw new TypeError("Cannot call a class as a function")}(this,e)}var n,t,o;return n=e,o=[{key:"decode",value:function(e){var n=new m(e),t=l.decode(n.popBits(16));if(!p.decode(n.popBits(1)))return{value:v.decode(n.popBits(t)),bitLength:n.getPosition()};for(var o={},r=l.decode(n.popBits(12)),i=0;i<r;i++){var u=p.decode(n.popBits(1)),c=l.decode(n.popBits(16));if(u)for(var a=l.decode(n.popBits(16)),s=c;s<=a;s++)o[s]=!0;else o[c]=!0}return{value:o,bitLength:n.getPosition()}}},{key:"encode",value:function(e){var n="",t=Math.max.apply(Math,b(Object.keys(e)).concat([0]));n+=l.encode(t,16);for(var o="",r=[],i=[],u=1;u<=t;u++)o+=e[u]?"1":"0",e[u]&&(0==i.length&&i.push(u),e[u+1]||(i.push(u),r.push(i),i=[]));return 45*r.length<o.length?(n+=p.encode(!0,1),n+=l.encode(r.length,12),r.forEach((function(e){var t=e[0]!==e[1];n+=p.encode(t,1),n+=l.encode(e[0],16),t&&(n+=l.encode(e[1],16))}))):n+=p.encode(!1,1)+o,n}}],(t=null)&&k(n.prototype,t),o&&k(n,o),e}();function S(e,n){for(var t=0;t<n.length;t++){var o=n[t];o.enumerable=o.enumerable||!1,o.configurable=!0,"value"in o&&(o.writable=!0),Object.defineProperty(e,o.key,o)}}var E=function(){function e(){!function(e,n){if(!(e instanceof n))throw new TypeError("Cannot call a class as a function")}(this,e)}var n,t,r;return n=e,r=[{key:"deserialize",value:function(n){var t=new m(o.decode(n));return e.decodeFields(e.fieldMap,t)}},{key:"decodeFields",value:function(n,t){var o={};return n.forEach((function(n){if(n.children)o[n.key]=e.decodeFields(n.children,t);else if(void 0===n.bitLength){var r=n.codec.decode(t.peekToEnd());o[n.key]=r.value,t.advance(r.bitLength)}else o[n.key]=n.codec.decode(t.popBits(n.bitLength))})),o}},{key:"serialize",value:function(n){var t=e.encodeFields(e.fieldMap,n);return o.encode(t)}},{key:"encodeFields",value:function(n,t){var o="";return n.forEach((function(n){n.children?o+=e.encodeFields(n.children,t[n.key]):void 0===n.bitLength?o+=n.codec.encode(t[n.key]):o+=n.codec.encode(t[n.key],n.bitLength)})),o}}],(t=null)&&S(n.prototype,t),r&&S(n,r),e}();E.fieldMap=[{key:"version",codec:l,bitLength:6},{key:"created",codec:c,bitLength:36},{key:"lastUpdated",codec:c,bitLength:36},{key:"cmpId",codec:l,bitLength:12},{key:"cmpVersion",codec:l,bitLength:12},{key:"consentScreen",codec:l,bitLength:6},{key:"consentLanguage",codec:f,bitLength:12},{key:"vendorListVersion",codec:l,bitLength:12},{key:"tcfPolicyVersion",codec:l,bitLength:6},{key:"isServiceSpecific",codec:p,bitLength:1},{key:"useNonStandardStacks",codec:p,bitLength:1},{key:"specialFeatureOptIns",codec:v,bitLength:12},{key:"purpose",children:[{key:"consents",codec:v,bitLength:24},{key:"legitimateInterests",codec:v,bitLength:24}]},{key:"purposeOneTreatment",codec:p,bitLength:1},{key:"publisherCC",codec:f,bitLength:12},{key:"vendor",children:[{key:"consents",codec:w},{key:"legitimateInterests",codec:w}]},{key:"numPubRestrictions",codec:l,bitLength:12}];var C=function(e){return e&&decodeURIComponent(document.cookie.replace(new RegExp("(?:(?:^|.*;)\\s*"+encodeURIComponent(e).replace(/[\-\.\+\*]/g,"\\$&")+"\\s*\\=\\s*([^;]*).*$)|^.*$"),"$1"))||null},L=r(843);var A=function e(n){!function(e,n){if(!(e instanceof n))throw new TypeError("Cannot call a class as a function")}(this,e),this.version=2,this.created=new Date,this.lastUpdated=this.created,this.cmpId=L.zq,this.cmpVersion=1,this.consentScreen=0,this.consentLanguage=n.consentLanguage,this.vendorListVersion=n.gvlVersion,this.tcfPolicyVersion=2,this.isServiceSpecific=!0,this.useNonStandardStacks=!1,this.specialFeatureOptIns={},this.purpose={consents:{},legitimateInterests:L.hA},this.purposeOneTreatment=!0,this.publisherCC="US",this.vendor={consents:{},legitimateInterests:n.vendorsLegInterest},this.numPubRestrictions=0};function P(e,n){for(var t=0;t<n.length;t++){var o=n[t];o.enumerable=o.enumerable||!1,o.configurable=!0,"value"in o&&(o.writable=!0),Object.defineProperty(e,o.key,o)}}new(function(){function t(){!function(e,n){if(!(e instanceof n))throw new TypeError("Cannot call a class as a function")}(this,t),this.config=this.loadConfiguration(),r.p=this.config.modulePath,this.acceptAllHandler=this.acceptAllHandler.bind(this),this.rejectAllHandler=this.rejectAllHandler.bind(this),this.saveHandler=this.saveHandler.bind(this),this.fetchGlobalVendorListHandler=this.fetchGlobalVendorListHandler.bind(this),this.model={},this.listeners=[],this.eventStatus=L.d6.UI_SHOWN,this.displayStatus=L.Uq.VISIBLE,this.cmpStatus=L.j4.LOADED,this.init()}var i,l,u;return i=t,(l=[{key:"init",value:function(){var e=this,n=C(L.B$);n?(this.model=E.deserialize(n),this.eventStatus=L.d6.LOADED,this.displayStatus=L.Uq.DISABLED):(this.model=new A(this.config),document.addEventListener("DOMContentLoaded",(function(){e.renderApp(L.pM.BANNER),e.bumpStat("wordads_cmp_view","no_cookie")})));var t=window.__tcfapi("getQueue");window.__tcfapi=this.commandHandler.bind(this),t.forEach((function(n){window.__tcfapi.apply(e,n)})),this.notify(this.model)}},{key:"loadConfiguration",value:function(){var e=JSON.parse(document.getElementById("cmp-configuration").innerText);return{gvlVersion:parseInt(e.gvlVersion),consentLanguage:e.consentLanguage,locale:e.locale,vendorsAll:w.decode(o.decode(e.vendorsAll)).value,vendorsLegInterest:w.decode(o.decode(e.vendorsLegInterest)).value,ajaxNonce:e.ajaxNonce,modulePath:e.modulePath,gvlPath:e.gvlPath,_:e._}}},{key:"getAppContainer",value:function(){var e="cmp-app-container",n=document.getElementById(e);return n||((n=document.createElement("div")).id=e,document.body.appendChild(n),n)}},{key:"renderApp",value:function(t){var o=arguments.length>1&&void 0!==arguments[1]&&arguments[1],i=(0,n.Vo)((function(){return r.e(143).then(r.bind(r,334))}));(0,e.sY)((0,e.h)(n.n4,null,(0,e.h)(i,{model:this.model,acceptAll:this.acceptAllHandler,rejectAll:this.rejectAllHandler,save:this.saveHandler,fetchGlobalVendorList:this.fetchGlobalVendorListHandler,path:this.config.modulePath,_:this.config._,mode:t,closable:o})),this.getAppContainer())}},{key:"showUi",value:function(){this.displayStatus!==L.Uq.VISIBLE&&(this.renderApp(L.pM.MODAL,!0),this.bumpStat("wordads_cmp_view","cookie"))}},{key:"bumpStat",value:function(e,n){var t={};t["x_"+e]=n,window._stq=window._stq||[],window._stq.push(["extra",t])}},{key:"setConsentCookie",value:function(e){var n,t=new XMLHttpRequest;t.open("POST","/wp-admin/admin-ajax.php",!0),t.setRequestHeader("Content-Type","application/x-www-form-urlencoded; charset=UTF-8"),(e=null!==(n=e)&&void 0!==n?n:{}).action="gdpr_set_consent",e.security=this.config.ajaxNonce,e.consent=E.serialize(this.model);var o=Object.keys(e).map((function(n){return n+"="+encodeURIComponent(e[n])})).join("&");t.send(o)}},{key:"fetchGlobalVendorListHandler",value:function(){var e=this,n=new XMLHttpRequest;return new Promise((function(t,o){n.onreadystatechange=function(){4===n.readyState&&(200===this.status?t(JSON.parse(this.response)):o({status:n.status,statusText:n.statusText}))},n.open("GET",e.config.gvlPath,!0),n.send()}))}},{key:"acceptAllHandler",value:function(){this.model.vendorListVersion=this.config.gvlVersion,this.model.lastUpdated=new Date,this.model.purpose={consents:L.a3,legitimateInterests:L.hA},this.model.vendor={consents:this.config.vendorsAll,legitimateInterests:this.config.vendorsLegInterest},this.eventStatus=L.d6.USER_ACTION_COMPLETE,this.displayStatus=L.Uq.HIDDEN,this.notify(),this.setConsentCookie({type:"accept_all"})}},{key:"rejectAllHandler",value:function(){this.model.vendorListVersion=this.config.gvlVersion,this.model.lastUpdated=new Date,this.model.purpose={consents:{},legitimateInterests:L.hA},this.model.vendor={consents:{},legitimateInterests:this.config.vendorsLegInterest},this.eventStatus=L.d6.USER_ACTION_COMPLETE,this.displayStatus=L.Uq.HIDDEN,this.notify(),this.setConsentCookie({type:"reject_all"})}},{key:"saveHandler",value:function(e){this.model=e,this.model.vendorListVersion=this.config.gvlVersion,this.model.lastUpdated=new Date,this.eventStatus=L.d6.USER_ACTION_COMPLETE,this.displayStatus=L.Uq.HIDDEN,this.notify(),this.setConsentCookie({type:"custom"})}},{key:"getTCData",value:function(){return{tcString:E.serialize(this.model),tcfPolicyVersion:this.model.tcfPolicyVersion,cmpId:this.model.cmpId,cmpVersion:this.model.cmpVersion,gdprApplies:!0,eventStatus:this.eventStatus,cmpStatus:this.cmpStatus,isServiceSpecific:this.model.isServiceSpecific,useNonStandardStacks:this.model.useNonStandardStacks,publisherCC:this.model.publisherCC,purposeOneTreatment:this.model.purposeOneTreatment,purpose:this.model.purpose,vendor:this.model.vendor,specialFeatureOptIns:this.model.specialFeatureOptIns}}},{key:"notify",value:function(){var e=this.getTCData();this.listeners.forEach((function(n){e.listenerId=n.listenerId,n.callback(e,!0)}))}},{key:"addListener",value:function(e){var n=Math.floor(999999*Math.random()),t={listenerId:n,callback:e};return this.listeners.push(t),n}},{key:"removeListener",value:function(e){this.listeners=this.listeners.filter((function(n){return n.listenerId!==e}))}},{key:"commandHandler",value:function(){var e=arguments.length<=0?void 0:arguments[0],n=arguments.length<=2?void 0:arguments[2],t=arguments.length<=3?void 0:arguments[3];if("ping"===e){var o={gdprApplies:!0,cmpLoaded:!0,cmpStatus:this.cmpStatus,displayStatus:this.displayStatus,apiVersion:"2",cmpVersion:this.model.cmpVersion,cmpId:this.model.cmpId,gvlVersion:this.model.vendorListVersion,tcfPolicyVersion:this.model.tcfPolicyVersion};"function"==typeof n&&n(o,!0)}else if("addEventListener"===e){var r=this.addListener(n),i={listenerId:r};n(i,!0)}else"removeEventListener"===e?(this.removeListener(t),n(!0)):"getTCData"===e?"function"==typeof n&&n(this.getTCData(),!0):"showUi"===e&&this.showUi()}}])&&P(i.prototype,l),u&&P(i,u),t}())}()}();;
/* globals JSON */
( function () {
	var eventName = 'wpcom_masterbar_click';

	var linksTracksEvents = {
		// top level items
		'wp-admin-bar-blog'                        : 'my_sites',
		'wp-admin-bar-newdash'                     : 'reader',
		'wp-admin-bar-ab-new-post'                 : 'write_button',
		'wp-admin-bar-my-account'                  : 'my_account',
		'wp-admin-bar-notes'                       : 'notifications',
		// my sites - top items
		'wp-admin-bar-switch-site'                 : 'my_sites_switch_site',
		'wp-admin-bar-blog-info'                   : 'my_sites_site_info',
		'wp-admin-bar-site-view'                   : 'my_sites_view_site',
		'wp-admin-bar-blog-stats'                  : 'my_sites_site_stats',
		'wp-admin-bar-plan'                        : 'my_sites_plan',
		'wp-admin-bar-plan-badge'                  : 'my_sites_plan_badge',
		// my sites - manage
		'wp-admin-bar-edit-page'                   : 'my_sites_manage_site_pages',
		'wp-admin-bar-new-page-badge'              : 'my_sites_manage_add_page',
		'wp-admin-bar-edit-post'                   : 'my_sites_manage_blog_posts',
		'wp-admin-bar-new-post-badge'              : 'my_sites_manage_add_post',
		'wp-admin-bar-edit-attachment'             : 'my_sites_manage_media',
		'wp-admin-bar-new-attachment-badge'        : 'my_sites_manage_add_media',
		'wp-admin-bar-comments'                    : 'my_sites_manage_comments',
		'wp-admin-bar-edit-jetpack-testimonial'    : 'my_sites_manage_testimonials',
		'wp-admin-bar-new-jetpack-testimonial'     : 'my_sites_manage_add_testimonial',
		'wp-admin-bar-edit-jetpack-portfolio'      : 'my_sites_manage_portfolio',
		'wp-admin-bar-new-jetpack-portfolio'       : 'my_sites_manage_add_portfolio',
		// my sites - personalize
		'wp-admin-bar-themes'                      : 'my_sites_personalize_themes',
		'wp-admin-bar-cmz'                         : 'my_sites_personalize_themes_customize',
		// my sites - configure
		'wp-admin-bar-sharing'                     : 'my_sites_configure_sharing',
		'wp-admin-bar-people'                      : 'my_sites_configure_people',
		'wp-admin-bar-people-add'                  : 'my_sites_configure_people_add_button',
		'wp-admin-bar-plugins'                     : 'my_sites_configure_plugins',
		'wp-admin-bar-domains'                     : 'my_sites_configure_domains',
		'wp-admin-bar-domains-add'                 : 'my_sites_configure_add_domain',
		'wp-admin-bar-blog-settings'               : 'my_sites_configure_settings',
		'wp-admin-bar-legacy-dashboard'            : 'my_sites_configure_wp_admin',
		// reader
		'wp-admin-bar-followed-sites'              : 'reader_followed_sites',
		'wp-admin-bar-reader-followed-sites-manage': 'reader_manage_followed_sites',
		'wp-admin-bar-discover-discover'           : 'reader_discover',
		'wp-admin-bar-discover-search'             : 'reader_search',
		'wp-admin-bar-my-activity-my-likes'        : 'reader_my_likes',
		// account
		'wp-admin-bar-user-info'                   : 'my_account_user_name',
		// account - profile
		'wp-admin-bar-my-profile'                  : 'my_account_profile_my_profile',
		'wp-admin-bar-account-settings'            : 'my_account_profile_account_settings',
		'wp-admin-bar-billing'                     : 'my_account_profile_manage_purchases',
		'wp-admin-bar-security'                    : 'my_account_profile_security',
		'wp-admin-bar-notifications'               : 'my_account_profile_notifications',
		// account - special
		'wp-admin-bar-get-apps'                    : 'my_account_special_get_apps',
		'wp-admin-bar-next-steps'                  : 'my_account_special_next_steps',
		'wp-admin-bar-help'                        : 'my_account_special_help',
	};

	var notesTracksEvents = {
		openSite: function ( data ) {
			return {
				clicked: 'masterbar_notifications_panel_site',
				site_id: data.siteId
			};
		},
		openPost: function ( data ) {
			return {
				clicked: 'masterbar_notifications_panel_post',
				site_id: data.siteId,
				post_id: data.postId
			};
		},
		openComment: function ( data ) {
			return {
				clicked: 'masterbar_notifications_panel_comment',
				site_id: data.siteId,
				post_id: data.postId,
				comment_id: data.commentId
			};
		}
	};

	// Element.prototype.matches as a standalone function, with old browser fallback
	function matches( node, selector ) {
		if ( ! node ) {
			return undefined;
		}

		if ( ! Element.prototype.matches && ! Element.prototype.msMatchesSelector ) {
			throw new Error( 'Unsupported browser' );
		}

		return Element.prototype.matches ? node.matches( selector ) : node.msMatchesSelector( selector );
	}

	// Element.prototype.closest as a standalone function, with old browser fallback
	function closest( node, selector ) {
		if ( ! node ) {
			return undefined;
		}

		if ( Element.prototype.closest ) {
			return node.closest( selector );
		}

		do {
			if ( matches( node, selector ) ) {
				return node;
			}

			node = node.parentElement || node.parentNode;
		} while ( node !== null && node.nodeType === 1 );

		return null;
	}

	function recordTracksEvent( eventProps ) {
		eventProps = eventProps || {};
		window._tkq = window._tkq || [];
		window._tkq.push( [ 'recordEvent', eventName, eventProps ] );
	}

	function parseJson( s, defaultValue ) {
		try {
			return JSON.parse( s );
		} catch ( e ) {
			return defaultValue;
		}
	}

	function createTrackableLinkEventHandler( link ) {
		return function () {
			var parent = closest( link, 'li' );

			if ( ! parent ) {
				return;
			}

			var trackingId = link.getAttribute( 'ID' ) || parent.getAttribute( 'ID' );

			if ( ! linksTracksEvents.hasOwnProperty( trackingId ) ) {
				return;
			}

			var eventProps = { 'clicked': linksTracksEvents[ trackingId ] };
			recordTracksEvent( eventProps );
		}
	}

	function init() {
		var trackableLinkSelector = '.mb-trackable .ab-item:not(div),' +
			'#wp-admin-bar-notes .ab-item,' +
			'#wp-admin-bar-user-info .ab-item,' +
			'.mb-trackable .ab-secondary';

		var trackableLinks = document.querySelectorAll( trackableLinkSelector );

		for ( var i = 0; i < trackableLinks.length; i++ ) {
			var link = trackableLinks[ i ];
			var handler = createTrackableLinkEventHandler( link );

			link.addEventListener( 'click', handler );
			link.addEventListener( 'touchstart', handler );
		}
	}

	if ( document.readyState === 'loading' ) {
		document.addEventListener( 'DOMContentLoaded', init );
	} else {
		init();
	}

	// listen for postMessage events from the notifications iframe
	window.addEventListener( 'message', function ( event ) {
		if ( event.origin !== 'https://widgets.wp.com' ) {
			return;
		}

		var data = ( typeof event.data === 'string' ) ? parseJson( event.data, {} ) : event.data;
		if ( data.type !== 'notesIframeMessage' ) {
			return;
		}

		var eventData = notesTracksEvents[ data.action ];
		if ( ! eventData ) {
			return;
		}

		recordTracksEvent( eventData( data ) );
	}, false );

} )();
;
/*! This file is auto-generated */
!function(c,d){"use strict";var e=!1,n=!1;if(d.querySelector)if(c.addEventListener)e=!0;if(c.wp=c.wp||{},!c.wp.receiveEmbedMessage)if(c.wp.receiveEmbedMessage=function(e){var t=e.data;if(t)if(t.secret||t.message||t.value)if(!/[^a-zA-Z0-9]/.test(t.secret)){for(var r,a,i,s=d.querySelectorAll('iframe[data-secret="'+t.secret+'"]'),n=d.querySelectorAll('blockquote[data-secret="'+t.secret+'"]'),o=0;o<n.length;o++)n[o].style.display="none";for(o=0;o<s.length;o++)if(r=s[o],e.source===r.contentWindow){if(r.removeAttribute("style"),"height"===t.message){if(1e3<(i=parseInt(t.value,10)))i=1e3;else if(~~i<200)i=200;r.height=i}if("link"===t.message)if(a=d.createElement("a"),i=d.createElement("a"),a.href=r.getAttribute("src"),i.href=t.value,i.host===a.host)if(d.activeElement===r)c.top.location.href=t.value}}},e)c.addEventListener("message",c.wp.receiveEmbedMessage,!1),d.addEventListener("DOMContentLoaded",t,!1),c.addEventListener("load",t,!1);function t(){if(!n){n=!0;for(var e,t,r=-1!==navigator.appVersion.indexOf("MSIE 10"),a=!!navigator.userAgent.match(/Trident.*rv:11\./),i=d.querySelectorAll("iframe.wp-embedded-content"),s=0;s<i.length;s++){if(!(e=i[s]).getAttribute("data-secret"))t=Math.random().toString(36).substr(2,10),e.src+="#?secret="+t,e.setAttribute("data-secret",t);if(r||a)(t=e.cloneNode(!0)).removeAttribute("security"),e.parentNode.replaceChild(t,e)}}}}(window,document);;
/* globals related_posts_js_options */

/**
 * Load related posts
 */
( function () {
	'use strict';

	var jprp = {
		response: null,

		/**
		 * Utility get related posts JSON endpoint from URLs
		 *
		 * @param  {string} URL (optional)
		 * @return {string} Endpoint URL
		 */
		getEndpointURL: function ( URL ) {
			var locationObject,
				is_customizer =
					'undefined' !== typeof wp &&
					wp.customize &&
					wp.customize.settings &&
					wp.customize.settings.url &&
					wp.customize.settings.url.self;

			// If we're in Customizer, write the correct URL.
			if ( is_customizer ) {
				locationObject = document.createElement( 'a' );
				locationObject.href = wp.customize.settings.url.self;
			} else {
				locationObject = document.location;
			}

			if ( 'string' === typeof URL && URL.match( /^https?:\/\// ) ) {
				locationObject = document.createElement( 'a' );
				locationObject.href = URL;
			}

			var args = 'relatedposts=1';
			var relatedPosts = document.querySelector( '#jp-relatedposts' );
			if ( relatedPosts.hasAttribute( 'data-exclude' ) ) {
				args += '&relatedposts_exclude=' + relatedPosts.getAttribute( 'data-exclude' );
			}

			if ( is_customizer ) {
				args += '&jetpackrpcustomize=1';
			}

			var pathname = locationObject.pathname;
			if ( '/' !== pathname[ 0 ] ) {
				pathname = '/' + pathname;
			}

			if ( '' === locationObject.search ) {
				return pathname + '?' + args;
			} else {
				return pathname + locationObject.search + '&' + args;
			}
		},

		getAnchor: function ( post, classNames ) {
			var anchorTitle = post.title;
			var anchor = document.createElement( 'a' );
			anchor.setAttribute( 'class', classNames );
			anchor.setAttribute( 'href', post.url );
			anchor.setAttribute( 'title', anchorTitle );
			anchor.setAttribute( 'data-origin', post.url_meta.origin );
			anchor.setAttribute( 'data-position', post.url_meta.position );

			if ( '' !== post.rel ) {
				anchor.setAttribute( 'rel', post.rel );
			}

			var div = document.createElement( 'div' );
			div.appendChild( anchor );

			var anchorHTML = div.innerHTML;
			return [ anchorHTML.substring( 0, anchorHTML.length - 4 ), '</a>' ];
		},

		generateMinimalHtml: function ( posts, options ) {
			var self = this;
			var html = '';

			posts.forEach( function ( post, index ) {
				var anchor = self.getAnchor( post, 'jp-relatedposts-post-a' );
				var classes = 'jp-relatedposts-post jp-relatedposts-post' + index;

				if ( post.classes.length > 0 ) {
					classes += ' ' + post.classes.join( ' ' );
				}

				html +=
					'<p class="' +
					classes +
					'" data-post-id="' +
					post.id +
					'" data-post-format="' +
					post.format +
					'">';
				html +=
					'<span class="jp-relatedposts-post-title">' +
					anchor[ 0 ] +
					post.title +
					anchor[ 1 ] +
					'</span>';
				if ( options.showDate ) {
					html +=
						'<time class="jp-relatedposts-post-date" datetime="' +
						post.date +
						'">' +
						post.date +
						'</time>';
				}
				if ( options.showContext ) {
					html += '<span class="jp-relatedposts-post-context">' + post.context + '</span>';
				}
				html += '</p>';
			} );
			return (
				'<div class="jp-relatedposts-items jp-relatedposts-items-minimal jp-relatedposts-' +
				options.layout +
				' ">' +
				html +
				'</div>'
			);
		},

		generateVisualHtml: function ( posts, options ) {
			var self = this;
			var html = '';

			posts.forEach( function ( post, index ) {
				var anchor = self.getAnchor( post, 'jp-relatedposts-post-a' );
				var classes = 'jp-relatedposts-post jp-relatedposts-post' + index;

				if ( post.classes.length > 0 ) {
					classes += ' ' + post.classes.join( ' ' );
				}

				if ( ! post.img.src ) {
					classes += ' jp-relatedposts-post-nothumbs';
				} else {
					classes += ' jp-relatedposts-post-thumbs';
				}

				var dummyContainer = document.createElement( 'p' );
				dummyContainer.innerHTML = post.excerpt;
				var excerpt = dummyContainer.textContent;

				html +=
					'<div class="' +
					classes +
					'" data-post-id="' +
					post.id +
					'" data-post-format="' +
					post.format +
					'">';
				if ( post.img.src ) {
					html +=
						anchor[ 0 ] +
						'<img class="jp-relatedposts-post-img" src="' +
						post.img.src +
						'" width="' +
						post.img.width +
						'" alt="' +
						post.img.alt_text +
						'" />' +
						anchor[ 1 ];
				} else {
					var anchor_overlay = self.getAnchor(
						post,
						'jp-relatedposts-post-a jp-relatedposts-post-aoverlay'
					);
					html += anchor_overlay[ 0 ] + anchor_overlay[ 1 ];
				}
				html +=
					'<' +
					related_posts_js_options.post_heading +
					' class="jp-relatedposts-post-title">' +
					anchor[ 0 ] +
					post.title +
					anchor[ 1 ] +
					'</' +
					related_posts_js_options.post_heading +
					'>';
				html += '<p class="jp-relatedposts-post-excerpt">' + excerpt + '</p>';
				if ( options.showDate ) {
					html +=
						'<time class="jp-relatedposts-post-date" datetime="' +
						post.date +
						'">' +
						post.date +
						'</time>';
				}
				if ( options.showContext ) {
					html += '<p class="jp-relatedposts-post-context">' + post.context + '</p>';
				}
				html += '</div>';
			} );
			return (
				'<div class="jp-relatedposts-items jp-relatedposts-items-visual jp-relatedposts-' +
				options.layout +
				' ">' +
				html +
				'</div>'
			);
		},

		/**
		 * We want to set a max height on the excerpt however we want to set
		 * this according to the natual pacing of the page as we never want to
		 * cut off a line of text in the middle so we need to do some detective
		 * work.
		 */
		setVisualExcerptHeights: function () {
			var elements = document.querySelectorAll(
				'#jp-relatedposts .jp-relatedposts-post-nothumbs .jp-relatedposts-post-excerpt'
			);

			if ( ! elements.length ) {
				return;
			}

			var firstElementStyles = getComputedStyle( elements[ 0 ] );

			var fontSize = parseInt( firstElementStyles.fontSize, 10 );
			var lineHeight = parseInt( firstElementStyles.lineHeight, 10 );

			// Show 5 lines of text
			for ( var i = 0; i < elements.length; i++ ) {
				elements[ i ].style.maxHeight = ( 5 * lineHeight ) / fontSize + 'em';
			}
		},

		getTrackedUrl: function ( anchor ) {
			var args = 'relatedposts_hit=1';
			args += '&relatedposts_origin=' + anchor.getAttribute( 'data-origin' );
			args += '&relatedposts_position=' + anchor.getAttribute( 'data-position' );

			var pathname = anchor.pathname;
			if ( '/' !== pathname[ 0 ] ) {
				pathname = '/' + pathname;
			}

			if ( '' === anchor.search ) {
				return pathname + '?' + args;
			} else {
				return pathname + anchor.search + '&' + args;
			}
		},

		cleanupTrackedUrl: function () {
			if ( 'function' !== typeof history.replaceState ) {
				return;
			}

			var cleaned_search = document.location.search.replace(
				/\brelatedposts_[a-z]+=[0-9]*&?\b/gi,
				''
			);
			if ( '?' === cleaned_search ) {
				cleaned_search = '';
			}
			if ( document.location.search !== cleaned_search ) {
				history.replaceState( {}, document.title, document.location.pathname + cleaned_search );
			}
		},
	};

	function afterPostsHaveLoaded() {
		jprp.setVisualExcerptHeights();
		var posts = document.querySelectorAll( '#jp-relatedposts a.jp-relatedposts-post-a' );

		Array.prototype.forEach.call( posts, function ( post ) {
			document.addEventListener( 'click', function () {
				post.href = jprp.getTrackedUrl( post );
			} );
		} );
	}

	/**
	 * Initialize Related Posts.
	 */
	function startRelatedPosts() {
		jprp.cleanupTrackedUrl();

		var endpointURL = jprp.getEndpointURL();
		var relatedPosts = document.querySelector( '#jp-relatedposts' );

		if ( document.querySelectorAll( '#jp-relatedposts .jp-relatedposts-post' ).length ) {
			afterPostsHaveLoaded();
			return;
		}

		var request = new XMLHttpRequest();
		request.open( 'GET', endpointURL, true );
		request.setRequestHeader( 'x-requested-with', 'XMLHttpRequest' );

		request.onreadystatechange = function () {
			if ( this.readyState === XMLHttpRequest.DONE && this.status === 200 ) {
				try {
					var response = JSON.parse( request.responseText );

					if ( 0 === response.items.length || 0 === relatedPosts.length ) {
						return;
					}

					jprp.response = response;

					var html,
						showThumbnails,
						options = {};

					if ( 'undefined' !== typeof wp && wp.customize ) {
						showThumbnails = wp.customize.instance( 'jetpack_relatedposts[show_thumbnails]' ).get();
						options.showDate = wp.customize.instance( 'jetpack_relatedposts[show_date]' ).get();
						options.showContext = wp.customize
							.instance( 'jetpack_relatedposts[show_context]' )
							.get();
						options.layout = wp.customize.instance( 'jetpack_relatedposts[layout]' ).get();
					} else {
						showThumbnails = response.show_thumbnails;
						options.showDate = response.show_date;
						options.showContext = response.show_context;
						options.layout = response.layout;
					}

					html = ! showThumbnails
						? jprp.generateMinimalHtml( response.items, options )
						: jprp.generateVisualHtml( response.items, options );

					var div = document.createElement( 'div' );
					relatedPosts.appendChild( div );
					div.outerHTML = html;

					if ( options.showDate ) {
						var dates = relatedPosts.querySelectorAll( '.jp-relatedposts-post-date' );

						Array.prototype.forEach.call( dates, function ( date ) {
							date.style.display = 'block';
						} );
					}

					relatedPosts.style.display = 'block';
					afterPostsHaveLoaded();
				} catch ( error ) {
					// Do nothing
				}
			}
		};

		request.send();
	}

	function init() {
		if ( 'undefined' !== typeof wp && wp.customize ) {
			if ( wp.customize.selectiveRefresh ) {
				wp.customize.selectiveRefresh.bind( 'partial-content-rendered', function ( placement ) {
					if ( 'jetpack_relatedposts' === placement.partial.id ) {
						startRelatedPosts();
					}
				} );
			}
			wp.customize.bind( 'preview-ready', startRelatedPosts );
		} else {
			startRelatedPosts();
		}
	}

	if ( document.readyState !== 'loading' ) {
		init();
	} else {
		document.addEventListener( 'DOMContentLoaded', init );
	}
} )();
;
var ak_js = document.getElementById( "ak_js" );

if ( ! ak_js ) {
	ak_js = document.createElement( 'input' );
	ak_js.setAttribute( 'id', 'ak_js' );
	ak_js.setAttribute( 'name', 'ak_js' );
	ak_js.setAttribute( 'type', 'hidden' );
}
else {
	ak_js.parentNode.removeChild( ak_js );
}

ak_js.setAttribute( 'value', ( new Date() ).getTime() );

var commentForm = document.getElementById( 'commentform' );

if ( commentForm ) {
	commentForm.appendChild( ak_js );
}
else {
	var replyRowContainer = document.getElementById( 'replyrow' );

	if ( replyRowContainer ) {
		var children = replyRowContainer.getElementsByTagName( 'td' );

		if ( children.length > 0 ) {
			children[0].appendChild( ak_js );
		}
	}
};
/* global WPCOM_sharing_counts, grecaptcha */

// NOTE: This file intentionally does not make use of polyfills or libraries,
// including jQuery. Please keep all code as IE11-compatible vanilla ES5, and
// ensure everything is inside an IIFE to avoid global namespace pollution.
// Code follows WordPress browser support guidelines. For an up to date list,
// see https://make.wordpress.org/core/handbook/best-practices/browser-support/

( function () {
	var currentScript = document.currentScript;
	var recaptchaScriptAdded = false;

	// -------------------------- UTILITY FUNCTIONS -------------------------- //

	// Helper function to load an external script.
	function loadScript( url ) {
		var script = document.createElement( 'script' );
		var prev = currentScript || document.getElementsByTagName( 'script' )[ 0 ];
		script.setAttribute( 'async', true );
		script.setAttribute( 'src', url );
		prev.parentNode.insertBefore( script, prev );
	}

	// Helper matches function (not a polyfill), compatible with IE 11.
	function matches( el, sel ) {
		if ( Element.prototype.matches ) {
			return el.matches( sel );
		}

		if ( Element.prototype.msMatchesSelector ) {
			return el.msMatchesSelector( sel );
		}
	}

	// Helper closest parent node function (not a polyfill) based on
	// https://developer.mozilla.org/en-US/docs/Web/API/Element/closest#Polyfill
	function closest( el, sel ) {
		if ( el.closest ) {
			return el.closest( sel );
		}

		var current = el;

		do {
			if ( matches( current, sel ) ) {
				return current;
			}
			current = current.parentElement || current.parentNode;
		} while ( current !== null && current.nodeType === 1 );

		return null;
	}

	// Helper function to iterate over a NodeList
	// (since IE 11 doesn't have NodeList.prototype.forEach)
	function forEachNode( list, fn ) {
		for ( var i = 0; i < list.length; i++ ) {
			var node = list[ i ];
			fn( node, i, list );
		}
	}

	// Helper function to remove a node from the DOM.
	function removeNode( node ) {
		if ( node && node.parentNode ) {
			node.parentNode.removeChild( node );
		}
	}

	// Helper functions to show/hide a node, and check its status.
	function hideNode( node ) {
		if ( node ) {
			node.style.display = 'none';
		}
	}

	function showNode( node ) {
		if ( node ) {
			node.style.removeProperty( 'display' );
		}
	}

	function isNodeHidden( node ) {
		return ! node || node.style.display === 'none';
	}

	// ------------------------------- CLASSES ------------------------------- //

	var PANE_SELECTOR = '.sharing-hidden .inner';
	var PANE_DATA_ATTR = 'data-sharing-more-button-id';

	// Implements a MoreButton class, which controls the lifecycle and behavior
	// of a "more" button and its dialog.
	function MoreButton( buttonEl ) {
		this.button = buttonEl;
		this.pane = closest( buttonEl, 'div' ).querySelector( PANE_SELECTOR );
		this.openedBy = null;
		this.recentlyOpenedByHover = false;

		MoreButton.instances.push( this );
		this.pane.setAttribute( PANE_DATA_ATTR, MoreButton.instances.length - 1 );

		this.attachHandlers();
	}

	// Keep a reference to each instance, so we can get back to it from the DOM.
	MoreButton.instances = [];

	// Delay time configs.
	MoreButton.hoverOpenDelay = 200;
	MoreButton.recentOpenDelay = 400;
	MoreButton.hoverCloseDelay = 300;

	// Use this to avoid creating new instances for buttons which already have one.
	MoreButton.instantiateOrReuse = function ( buttonEl ) {
		var pane = closest( buttonEl, 'div' ).querySelector( PANE_SELECTOR );
		var paneId = pane && pane.getAttribute( PANE_DATA_ATTR );

		var existingInstance = MoreButton.instances[ paneId ];
		if ( existingInstance ) {
			return existingInstance;
		}

		return new MoreButton( buttonEl );
	};

	// Retrieve a button instance from the pane DOM element.
	MoreButton.getButtonInstanceFromPane = function ( paneEl ) {
		var paneId = paneEl && paneEl.getAttribute( PANE_DATA_ATTR );
		return MoreButton.instances[ paneId ];
	};

	// Close all open More Button dialogs.
	MoreButton.closeAll = function () {
		for ( var i = 0; i < MoreButton.instances.length; i++ ) {
			MoreButton.instances[ i ].close();
		}
	};

	MoreButton.prototype.open = function () {
		var offset;
		var offsetParent;
		var parentOffset = [ 0, 0 ];

		function getOffsets( el ) {
			var rect = el.getBoundingClientRect();
			return [
				rect.left + ( window.scrollX || window.pageXOffset || 0 ),
				rect.top + ( window.scrollY || window.pageYOffset || 0 ),
			];
		}

		function getStyleValue( el, prop ) {
			return parseInt( getComputedStyle( el ).getPropertyValue( prop ) || 0 );
		}

		offset = getOffsets( this.button );
		offsetParent = this.button.offsetParent || document.documentElement;

		while (
			offsetParent &&
			( offsetParent === document.body || offsetParent === document.documentElement ) &&
			getComputedStyle( offsetParent ).getPropertyValue( 'position' ) === 'static'
		) {
			offsetParent = offsetParent.parentNode;
		}

		if ( offsetParent && offsetParent !== this.button && offsetParent.nodeType === 1 ) {
			parentOffset = getOffsets( offsetParent );
			parentOffset = [
				parentOffset[ 0 ] + getStyleValue( offsetParent, 'border-left-width' ),
				parentOffset[ 1 ] + getStyleValue( offsetParent, 'border-top-width' ),
			];
		}

		var positionLeft =
			offset[ 0 ] - parentOffset[ 0 ] - getStyleValue( this.button, 'margin-left' );
		var positionTop = offset[ 1 ] - parentOffset[ 1 ] - getStyleValue( this.button, 'margin-top' );

		this.pane.style.left = positionLeft + 'px';
		this.pane.style.top = positionTop + this.button.offsetHeight + 3 + 'px';

		showNode( this.pane );
	};

	MoreButton.prototype.close = function () {
		hideNode( this.pane );
		this.openedBy = null;
	};

	MoreButton.prototype.toggle = function () {
		if ( isNodeHidden( this.pane ) ) {
			this.open();
		} else {
			this.close();
		}
	};

	MoreButton.prototype.resetCloseTimer = function () {
		clearTimeout( this.closeTimer );
		this.closeTimer = setTimeout( this.close.bind( this ), MoreButton.hoverCloseDelay );
	};

	MoreButton.prototype.attachHandlers = function () {
		this.buttonClick = function ( event ) {
			event.preventDefault();
			event.stopPropagation();

			this.openedBy = 'click';
			clearTimeout( this.openTimer );
			clearTimeout( this.closeTimer );

			closeEmailDialog();

			if ( this.recentlyOpenedByHover ) {
				this.recentlyOpenedByHover = false;
				clearTimeout( this.hoverOpenTimer );
				this.open();
			} else {
				this.toggle();
			}
		}.bind( this );

		this.buttonEnter = function () {
			if ( ! this.openedBy ) {
				this.openTimer = setTimeout(
					function () {
						closeEmailDialog();
						this.open();
						this.openedBy = 'hover';
						this.recentlyOpenedByHover = true;
						this.hoverOpenTimer = setTimeout(
							function () {
								this.recentlyOpenedByHover = false;
							}.bind( this ),
							MoreButton.recentOpenDelay
						);
					}.bind( this ),
					MoreButton.hoverOpenDelay
				);
			}
			clearTimeout( this.closeTimer );
		}.bind( this );

		this.buttonLeave = function () {
			if ( this.openedBy === 'hover' ) {
				this.resetCloseTimer();
			}
			clearTimeout( this.openTimer );
		}.bind( this );

		this.paneEnter = function () {
			clearTimeout( this.closeTimer );
		}.bind( this );

		this.paneLeave = function () {
			if ( this.openedBy === 'hover' ) {
				this.resetCloseTimer();
			}
		}.bind( this );

		this.documentClick = function () {
			this.close();
		}.bind( this );

		this.button.addEventListener( 'click', this.buttonClick );
		document.addEventListener( 'click', this.documentClick );

		if ( document.ontouchstart === undefined ) {
			// Non-touchscreen device: use hover/mouseout with delay
			this.button.addEventListener( 'mouseenter', this.buttonEnter );
			this.button.addEventListener( 'mouseleave', this.buttonLeave );
			this.pane.addEventListener( 'mouseenter', this.paneEnter );
			this.pane.addEventListener( 'mouseleave', this.paneLeave );
		}
	};

	// ---------------------------- SHARE COUNTS ---------------------------- //

	if ( window.sharing_js_options && window.sharing_js_options.counts ) {
		var WPCOMSharing = {
			done_urls: [],
			get_counts: function () {
				var url, requests, id, service, service_request;

				if ( 'undefined' === typeof WPCOM_sharing_counts ) {
					return;
				}

				for ( url in WPCOM_sharing_counts ) {
					id = WPCOM_sharing_counts[ url ];

					if ( 'undefined' !== typeof WPCOMSharing.done_urls[ id ] ) {
						continue;
					}

					requests = {
						// Pinterest handles share counts for both http and https
						pinterest: [
							window.location.protocol +
								'//api.pinterest.com/v1/urls/count.json?callback=WPCOMSharing.update_pinterest_count&url=' +
								encodeURIComponent( url ),
						],
						// Facebook protocol summing has been shown to falsely double counts, so we only request the current URL
						facebook: [
							window.location.protocol +
								'//graph.facebook.com/?callback=WPCOMSharing.update_facebook_count&ids=' +
								encodeURIComponent( url ),
						],
					};

					for ( service in requests ) {
						if ( ! document.querySelector( 'a[data-shared=sharing-' + service + '-' + id + ']' ) ) {
							continue;
						}

						while ( ( service_request = requests[ service ].pop() ) ) {
							loadScript( service_request );
						}

						if ( window.sharing_js_options.is_stats_active ) {
							WPCOMSharing.bump_sharing_count_stat( service );
						}
					}

					WPCOMSharing.done_urls[ id ] = true;
				}
			},

			// get the version of the url that was stored in the dom
			get_permalink: function ( url ) {
				if ( 'https:' === window.location.protocol ) {
					url = url.replace( /^http:\/\//i, 'https://' );
				} else {
					url = url.replace( /^https:\/\//i, 'http://' );
				}

				return url;
			},
			update_facebook_count: function ( data ) {
				var url, permalink;

				if ( ! data ) {
					return;
				}

				for ( url in data ) {
					if (
						! Object.prototype.hasOwnProperty.call( data, url ) ||
						! data[ url ].share ||
						! data[ url ].share.share_count
					) {
						continue;
					}

					permalink = WPCOMSharing.get_permalink( url );

					if ( ! ( permalink in WPCOM_sharing_counts ) ) {
						continue;
					}

					WPCOMSharing.inject_share_count(
						'sharing-facebook-' + WPCOM_sharing_counts[ permalink ],
						data[ url ].share.share_count
					);
				}
			},
			update_pinterest_count: function ( data ) {
				if ( 'undefined' !== typeof data.count && data.count * 1 > 0 ) {
					WPCOMSharing.inject_share_count(
						'sharing-pinterest-' + WPCOM_sharing_counts[ data.url ],
						data.count
					);
				}
			},
			inject_share_count: function ( id, count ) {
				forEachNode( document.querySelectorAll( 'a[data-shared=' + id + '] > span' ), function (
					span
				) {
					var countNode = span.querySelector( '.share-count' );
					removeNode( countNode );
					var newNode = document.createElement( 'span' );
					newNode.className = 'share-count';
					newNode.textContent = WPCOMSharing.format_count( count );
					span.appendChild( newNode );
				} );
			},
			format_count: function ( count ) {
				if ( count < 1000 ) {
					return count;
				}
				if ( count >= 1000 && count < 10000 ) {
					return String( count ).substring( 0, 1 ) + 'K+';
				}
				return '10K+';
			},
			bump_sharing_count_stat: function ( service ) {
				new Image().src =
					document.location.protocol +
					'//pixel.wp.com/g.gif?v=wpcom-no-pv&x_sharing-count-request=' +
					service +
					'&r=' +
					Math.random();
			},
		};
		window.WPCOMSharing = WPCOMSharing;
	}

	// ------------------------ BUTTON FUNCTIONALITY ------------------------ //

	function shareIsEmail( val ) {
		return /^((([a-z]|\d|[!#\$%&'\*\+\-\/=\?\^_`{\|}~]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])+(\.([a-z]|\d|[!#\$%&'\*\+\-\/=\?\^_`{\|}~]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])+)*)|((\x22)((((\x20|\x09)*(\x0d\x0a))?(\x20|\x09)+)?(([\x01-\x08\x0b\x0c\x0e-\x1f\x7f]|\x21|[\x23-\x5b]|[\x5d-\x7e]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(\\([\x01-\x09\x0b\x0c\x0d-\x7f]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF]))))*(((\x20|\x09)*(\x0d\x0a))?(\x20|\x09)+)?(\x22)))@((([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])*([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])))\.)+(([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])*([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])))\.?$/i.test(
			val
		);
	}

	function closeEmailDialog() {
		var dialog = document.querySelector( '#sharing_email' );
		hideNode( dialog );
	}

	// Sharing initialization.
	// Will run immediately or on `DOMContentLoaded`, depending on current page status.
	function init() {
		// Move email dialog to end of body.
		var emailDialog = document.querySelector( '#sharing_email' );
		if ( emailDialog ) {
			document.body.appendChild( emailDialog );
		}

		WPCOMSharing_do();
	}
	if ( document.readyState !== 'loading' ) {
		init();
	} else {
		document.addEventListener( 'DOMContentLoaded', init );
	}

	// Set up sharing again whenever a new post loads, to pick up any new buttons.
	document.body.addEventListener( 'is.post-load', WPCOMSharing_do );

	// Set up sharing, updating counts and adding all button functionality.
	function WPCOMSharing_do() {
		if ( window.WPCOMSharing ) {
			window.WPCOMSharing.get_counts();
		}

		forEachNode( document.querySelectorAll( '.sharedaddy a' ), function ( anchor ) {
			var href = anchor.getAttribute( 'href' );
			if ( href && href.indexOf( 'share=' ) !== -1 && href.indexOf( '&nb=1' ) === -1 ) {
				anchor.setAttribute( 'href', href + '&nb=1' );
			}
		} );

		// Show hidden buttons

		// Touchscreen device: use click.
		// Non-touchscreen device: use click if not already appearing due to a hover event

		forEachNode( document.querySelectorAll( '.sharedaddy a.sharing-anchor' ), function (
			buttonEl
		) {
			MoreButton.instantiateOrReuse( buttonEl );
		} );

		if ( document.ontouchstart !== undefined ) {
			document.body.classList.add( 'jp-sharing-input-touch' );
		}

		// Add click functionality
		forEachNode( document.querySelectorAll( '.sharedaddy ul' ), function ( group ) {
			if ( group.getAttribute( 'data-sharing-events-added' ) === 'true' ) {
				return;
			}
			group.setAttribute( 'data-sharing-events-added', 'true' );

			var printUrl = function ( uniqueId, urlToPrint ) {
				var iframe = document.createElement( 'iframe' );
				iframe.setAttribute(
					'style',
					'position:fixed; top:100; left:100; height:1px; width:1px; border:none;'
				);
				iframe.setAttribute( 'id', 'printFrame-' + uniqueId );
				iframe.setAttribute( 'name', iframe.getAttribute( 'id' ) );
				iframe.setAttribute( 'src', urlToPrint );
				iframe.setAttribute(
					'onload',
					'frames["printFrame-' +
						uniqueId +
						'"].focus();frames["printFrame-' +
						uniqueId +
						'"].print();'
				);
				document.body.appendChild( iframe );
			};

			// Print button
			forEachNode( group.querySelectorAll( 'a.share-print' ), function ( printButton ) {
				printButton.addEventListener( 'click', function ( event ) {
					event.preventDefault();
					event.stopPropagation();

					var ref = printButton.getAttribute( 'href' ) || '';
					var doPrint = function () {
						if ( ref.indexOf( '#print' ) === -1 ) {
							var uid = new Date().getTime();
							printUrl( uid, ref );
						} else {
							window.print();
						}
					};

					// Is the button in a dropdown?
					var pane = closest( printButton, PANE_SELECTOR );
					if ( pane ) {
						var moreButton = MoreButton.getButtonInstanceFromPane( pane );
						if ( moreButton ) {
							moreButton.close();
							doPrint();
						}
					} else {
						doPrint();
					}
				} );
			} );

			// Press This button
			forEachNode( group.querySelectorAll( 'a.share-press-this' ), function ( pressThisButton ) {
				pressThisButton.addEventListener( 'click', function ( event ) {
					event.preventDefault();
					event.stopPropagation();

					var s = '';

					if ( window.getSelection ) {
						s = window.getSelection();
					} else if ( document.getSelection ) {
						s = document.getSelection();
					} else if ( document.selection ) {
						s = document.selection.createRange().text;
					}

					if ( s ) {
						var href = pressThisButton.getAttribute( 'href' );
						pressThisButton.setAttribute( 'href', href + '&sel=' + encodeURI( s ) );
					}

					if (
						! window.open(
							pressThisButton.getAttribute( 'href' ),
							't',
							'toolbar=0,resizable=1,scrollbars=1,status=1,width=720,height=570'
						)
					) {
						document.location.href = pressThisButton.getAttribute( 'href' );
					}
				} );
			} );

			// Email button
			forEachNode( group.querySelectorAll( 'a.share-email' ), function ( emailButton ) {
				var dialog = document.querySelector( '#sharing_email' );

				emailButton.addEventListener( 'click', function ( event ) {
					event.preventDefault();
					event.stopPropagation();

					// Load reCAPTCHA if needed.
					if ( typeof grecaptcha !== 'object' && ! recaptchaScriptAdded ) {
						var configEl = document.querySelector( '.g-recaptcha' );

						if ( configEl && configEl.getAttribute( 'data-lazy' ) === 'true' ) {
							recaptchaScriptAdded = true;
							loadScript( decodeURI( configEl.getAttribute( 'data-url' ) ) );
						}
					}

					var url = emailButton.getAttribute( 'href' );
					var currentDomain = window.location.protocol + '//' + window.location.hostname + '/';
					if ( url.indexOf( currentDomain ) !== 0 ) {
						return true;
					}

					if ( ! isNodeHidden( dialog ) ) {
						closeEmailDialog();
						return;
					}

					removeNode( document.querySelector( '#sharing_email .response' ) );

					var form = document.querySelector( '#sharing_email form' );
					showNode( form );
					form.querySelector( 'input[type=submit]' ).removeAttribute( 'disabled' );
					showNode( form.querySelector( 'a.sharing_cancel' ) );

					// Reset reCATPCHA if exists.
					if (
						'object' === typeof grecaptcha &&
						'function' === typeof grecaptcha.reset &&
						window.___grecaptcha_cfg.count
					) {
						grecaptcha.reset();
					}

					// Show dialog
					var rect = emailButton.getBoundingClientRect();
					var scrollLeft = window.pageXOffset || document.documentElement.scrollLeft || 0;
					var scrollTop = window.pageYOffset || document.documentElement.scrollTop || 0;
					dialog.style.left = scrollLeft + rect.left + 'px';
					dialog.style.top = scrollTop + rect.top + rect.height + 'px';
					showNode( dialog );

					// Close all open More Button dialogs.
					MoreButton.closeAll();
				} );

				// Hook up other buttons
				dialog.querySelector( 'a.sharing_cancel' ).addEventListener( 'click', function ( event ) {
					event.preventDefault();
					event.stopPropagation();

					hideNode( dialog.querySelector( '.errors' ) );
					hideNode( dialog );
					hideNode( document.querySelector( '#sharing_background' ) );
				} );

				var submitButton = dialog.querySelector( 'input[type=submit]' );
				submitButton.addEventListener( 'click', function ( event ) {
					event.preventDefault();
					event.stopPropagation();

					var form = closest( submitButton, 'form' );
					var source_email_input = form.querySelector( 'input[name=source_email]' );
					var target_email_input = form.querySelector( 'input[name=target_email]' );

					// Disable buttons + enable loading icon
					submitButton.setAttribute( 'disabled', true );
					hideNode( form.querySelector( 'a.sharing_cancel' ) );
					forEachNode( form.querySelectorAll( 'img.loading' ), function ( img ) {
						showNode( img );
					} );

					hideNode( form.querySelector( '.errors' ) );

					forEachNode( form.querySelectorAll( '.error' ), function ( node ) {
						node.classList.remove( 'error' );
					} );

					if ( ! shareIsEmail( source_email_input.value ) ) {
						source_email_input.classList.add( 'error' );
					}

					if ( ! shareIsEmail( target_email_input.value ) ) {
						target_email_input.classList.add( 'error' );
					}

					if ( ! form.querySelector( '.error' ) ) {
						// Encode form data. This would be much easier if we could rely on URLSearchParams...
						var params = [];
						for ( var i = 0; i < form.elements.length; i++ ) {
							if ( form.elements[ i ].name ) {
								// Encode each form element into a URI-compatible string.
								var encoded =
									encodeURIComponent( form.elements[ i ].name ) +
									'=' +
									encodeURIComponent( form.elements[ i ].value );
								// In x-www-form-urlencoded, spaces should be `+`, not `%20`.
								params.push( encoded.replace( '%20', '+' ) );
							}
						}
						var data = params.join( '&' );

						// AJAX send the form
						var request = new XMLHttpRequest();
						request.open( 'POST', emailButton.getAttribute( 'href' ), true );
						request.setRequestHeader(
							'Content-Type',
							'application/x-www-form-urlencoded; charset=UTF-8'
						);
						request.setRequestHeader( 'x-requested-with', 'XMLHttpRequest' );

						request.onreadystatechange = function () {
							if ( this.readyState === XMLHttpRequest.DONE && this.status === 200 ) {
								forEachNode( form.querySelectorAll( 'img.loading' ), function ( img ) {
									hideNode( img );
								} );

								if ( this.response === '1' || this.response === '2' || this.response === '3' ) {
									showNode( dialog.querySelector( '.errors-' + this.response ) );
									dialog.querySelector( 'input[type=submit]' ).removeAttribute( 'disabled' );
									showNode( dialog.querySelector( 'a.sharing_cancel' ) );

									if ( typeof grecaptcha === 'object' && typeof grecaptcha.reset === 'function' ) {
										grecaptcha.reset();
									}
								} else {
									hideNode( form );
									var temp = document.createElement( 'div' );
									temp.innerHTML = this.response;
									dialog.appendChild( temp.firstChild );
									showNode( dialog.querySelector( 'a.sharing_cancel' ) );
									var closeButton = dialog.querySelector( '.response a.sharing_cancel' );
									if ( closeButton ) {
										closeButton.addEventListener( 'click', function ( event ) {
											event.preventDefault();
											event.stopPropagation();

											closeEmailDialog();
											hideNode( document.querySelector( '#sharing_background' ) );
										} );
									}
								}
							}
						};

						request.send( data );

						return;
					}

					forEachNode( dialog.querySelectorAll( 'img.loading' ), function ( img ) {
						hideNode( img );
					} );
					submitButton.removeAttribute( 'disabled' );
					showNode( dialog.querySelector( 'a.sharing_cancel' ) );
					forEachNode( dialog.querySelectorAll( '.errors-1' ), function ( error ) {
						showNode( error );
					} );
				} );
			} );
		} );

		forEachNode(
			document.querySelectorAll( 'li.share-email, li.share-custom a.sharing-anchor' ),
			function ( node ) {
				node.classList.add( 'share-service-visible' );
			}
		);
	}
} )();
;
