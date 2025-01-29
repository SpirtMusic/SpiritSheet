import QtQuick 2.9
import com.SpiritMusic.Poppler 1.0

ListView {
    id: pagesView

    property alias path: poppler.path
    property alias loaded: poppler.loaded
    property real zoom: 1.0
    property alias poppler: poppler
    property int count: poppler.pages.length
    property int currentPage: currentIndex
    property color searchHighlightColor: Qt.rgba(1, 1, .2, .4)
    property real dragStartX: 0
    property real dragThreshold: width * 0.2 // 20% of width for swipe threshold
    property bool isDragging: false
    property bool isAnimating: false
    property real targetX: 0
    // View mode properties
    property int viewMode: root.viewMode
    property bool isHorizontal: viewMode > 0
    property bool isBookMode: viewMode === 2
    property bool animatingScroll: false
    property int scrollDuration: 200
    // ListView properties
    clip: true
    spacing: isHorizontal ? 0 : 20
    orientation: isHorizontal ? ListView.Horizontal : ListView.Vertical
    cacheBuffer: isHorizontal ? width * 2 : height * 2
    pixelAligned: true
  flickableDirection: Flickable.VerticalFlick  // Only allow vertical flicking
interactive: !isHorizontal  // Disable ListView's default interaction in horizontal mode
boundsBehavior: isHorizontal ? Flickable.StopAtBounds : Flickable.DragAndOvershootBounds
    model: poppler.loaded ? (isBookMode ? Math.ceil(poppler.pages.length / 2) : poppler.pages.length) : 0
    //reuseItems: true
    displayMarginBeginning: isHorizontal ? width : height
    displayMarginEnd: isHorizontal ? width : height
    // Disable flickering for horizontal modes
    flickDeceleration: isHorizontal ? 10000 : 2500
    maximumFlickVelocity: isHorizontal ? 1000 : 4000
    onIsHorizontalChanged: {
        contentX = 0
        contentY = 0
        currentIndex = Math.floor(currentPage / (isBookMode ? 2 : 1))
    }
    populate: Transition {
        NumberAnimation {
            properties: "x,y"
            duration: isHorizontal ? 200 : 0  // No animation in scroll mode
            easing.type: Easing.OutCubic
        }
    }

    add: Transition {
        NumberAnimation {
            properties: isHorizontal ? "x" : "y"
            from: isHorizontal ? width : height
            duration: isHorizontal ? 200 : 0  // No animation in scroll mode
            easing.type: Easing.OutCubic
        }
    }

    remove: Transition {
        NumberAnimation {
            properties: isHorizontal ? "x" : "y"
            to: isHorizontal ? -width : -height
            duration: isHorizontal ? 200 : 0  // No animation in scroll mode
            easing.type: Easing.OutCubic
        }
    }

    onViewModeChanged: {
        currentIndex = Math.floor(currentPage / (isBookMode ? 2 : 1))
        contentX = 0
        contentY = 0
    }
    // Define signals
    signal errorOccurred(string message)
    signal searchNotFound
    signal searchRestartedFromTheBeginning

    // Search function
    function search(text) {
        if (!poppler.loaded) return

        if (text.length === 0) {
            __currentSearchTerm = ''
            __currentSearchResultIndex = -1
            __currentSearchResults = []
        } else if (text === __currentSearchTerm) {
            if (__currentSearchResultIndex < __currentSearchResults.length - 1) {
                __currentSearchResultIndex++
                __scrollTo(__currentSearchResult)
            } else {
                var page = __currentSearchResult.page
                __currentSearchResultIndex = -1
                __currentSearchResults = []
                if (page < count - 1) {
                    __search(page + 1, __currentSearchTerm)
                } else {
                    pagesView.searchRestartedFromTheBeginning()
                    __search(0, __currentSearchTerm)
                }
            }
        } else {
            __currentSearchTerm = text
            __currentSearchResultIndex = -1
            __currentSearchResults = []
            __search(currentPage, text)
        }
    }

    // Private properties
    property string __currentSearchTerm
    property int __currentSearchResultIndex: -1
    property var __currentSearchResults: []
    property var __currentSearchResult: __currentSearchResultIndex > -1 ?
                                            __currentSearchResults[__currentSearchResultIndex] :
                                            { page: -1, rect: Qt.rect(0,0,0,0) }

    // Poppler instance
    Poppler {
        id: poppler
        onLoadedChanged: {
            __updateCurrentPage()
            __currentSearchTerm = ''
            __currentSearchResultIndex = -1
            __currentSearchResults = []
        }
        onError: function(errorMessage) {
            pagesView.errorOccurred(errorMessage)
        }
    }

    // Private functions
    function __updateCurrentPage() {
        var p = pagesView.indexAt(pagesView.width / 2, pagesView.contentY + pagesView.height / 2)
        if (p === -1)
            p = pagesView.indexAt(pagesView.width / 2, pagesView.contentY + pagesView.height / 2 + pagesView.spacing)
        currentPage = p
    }

    function __search(startPage, text) {
        if (startPage >= count) {
            console.error('Start page index is larger than number of pages in document')
            return
        }

        function resultFound(page, result) {
            var searchResults = []
            for (var i = 0; i < result.length; ++i) {
                searchResults.push({ page: page, rect: result[i] })
            }
            __currentSearchResults = searchResults
            __currentSearchResultIndex = 0
            __scrollTo(__currentSearchResult)
        }

        var found = false
        for (var page = startPage; page < count; ++page) {
            var result = poppler.search(page, text)

            if (result.length > 0) {
                found = true
                resultFound(page, result)
                break
            }
        }

        if (!found) {
            for (page = 0; page < startPage; ++page) {
                result = poppler.search(page, text)

                if (result.length > 0) {
                    found = true
                    pagesView.searchRestartedFromTheBeginning()
                    resultFound(page, result)
                    break
                }
            }
        }

        if (!found) {
            pagesView.searchNotFound()
        }
    }

    function __goTo(destination) {
        pagesView.positionViewAtIndex(destination.page, ListView.Beginning)
        var pageHeight = poppler.pages[destination.page].size.height * zoom
        var scroll = Math.round(destination.top * pageHeight)
        pagesView.contentY += scroll
    }

    function __scrollTo(destination) {
        if (destination.page !== currentPage) {
            pagesView.positionViewAtIndex(destination.page, ListView.Beginning)
        }

        var i = pagesView.itemAt(pagesView.width / 2, pagesView.contentY + pagesView.height / 2)
        if (i === null)
            i = pagesView.itemAt(pagesView.width / 2, pagesView.contentY + pagesView.height / 2 + pagesView.spacing)

        var pageHeight = poppler.pages[destination.page].size.height * zoom
        var pageY = i.y - pagesView.contentY

        var bottomDistance = pagesView.height - (pageY + Math.round(destination.rect.bottom * pageHeight))
        var topDistance = pageY + Math.round(destination.rect.top * pageHeight)
        if (bottomDistance < 0) {
            pagesView.contentY -= bottomDistance - pagesView.spacing
        } else if (topDistance < 0) {
            pagesView.contentY += topDistance - pagesView.spacing
        }
    }



    // Content layout
    header: Item { height: 10 }
    footer: Item { height: 10 }

    // Connections for content changes
    Connections {
        target: pagesView
        function onContentYChanged() {
            __updateCurrentPage()
        }
    }

    // Delegate for pages
    delegate: Item {
        id: delegateItem
        width: isHorizontal ? pagesView.width : pagesView.width
        height: isHorizontal ? pagesView.height : contentHeight
        layer.enabled: true
        layer.smooth: true
        layer.samples: 4

        property real contentHeight: isBookMode ?
                                         Math.max(leftPage.implicitHeight || 0, rightPage.implicitHeight || 0) :
                                         (singlePage.implicitHeight || 0)

        // Single page mode
        Image {
            id: singlePage
            visible: !isBookMode
            anchors.horizontalCenter: parent.horizontalCenter
            cache: false
            smooth: false
            fillMode: isHorizontal ? Image.PreserveAspectFit : Image.Pad

            // Use the correct model data based on mode
            property var pageData: !isBookMode ? poppler.pages[index] : null

            sourceSize.width: pageData ? Math.round(pageData.size.width * zoom) : 0
            sourceSize.height: pageData ? Math.round(pageData.size.height * zoom) : 0
            source: pageData ? pageData.image : ""

            // Links for single page
            Repeater {
                model: singlePage.pageData ? singlePage.pageData.links : null
                delegate: MouseArea {
                    x: Math.round(modelData.rect.x * parent.width)
                    y: Math.round(modelData.rect.y * parent.height)
                    width: Math.round(modelData.rect.width * parent.width)
                    height: Math.round(modelData.rect.height * parent.height)
                    cursorShape: Qt.PointingHandCursor
                    onClicked: __goTo(modelData.destination)
                }
            }
        }

        // Book mode (two pages)
        Image {
            id: leftPage
            visible: isBookMode
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width / 2
            cache: false
            smooth: false
            fillMode: Image.PreserveAspectFit

            property var pageData: isBookMode && (index * 2) < poppler.pages.length ?
                                       poppler.pages[index * 2] : null

            sourceSize.width: pageData ? Math.round(pageData.size.width * zoom) : 0
            sourceSize.height: pageData ? Math.round(pageData.size.height * zoom) : 0
            source: pageData ? pageData.image : ""
        }

        Image {
            id: rightPage
            visible: isBookMode && (index * 2 + 1) < poppler.pages.length
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width / 2
            cache: false
            smooth: false
            fillMode: Image.PreserveAspectFit

            property var pageData: isBookMode && (index * 2 + 1) < poppler.pages.length ?
                                       poppler.pages[index * 2 + 1] : null

            sourceSize.width: pageData ? Math.round(pageData.size.width * zoom) : 0
            sourceSize.height: pageData ? Math.round(pageData.size.height * zoom) : 0
            source: pageData ? pageData.image : ""
        }
    }
    // Navigation functions
    function goToNextPage() {
         if (currentIndex < count - 1) {
             if (isHorizontal) {
                 currentIndex = currentIndex + 1
                 contentX = currentIndex * width
             } else {
                 currentIndex = currentIndex + 1
                 contentY += height
             }
         }
     }


    function goToPreviousPage() {
         if (currentIndex > 0) {
             if (isHorizontal) {
                 currentIndex = currentIndex - 1
                 contentX = currentIndex * width
             } else {
                 currentIndex = currentIndex - 1
                 contentY -= height
             }
         }
     }

    function positionViewAtIndex(index, mode) {
           if (index < 0 || index >= count) return

           currentIndex = index
           if (isHorizontal) {
               contentX = index * width
           } else {
               contentY = index * (height + spacing)
           }
       }



    function goToPage(pageNumber) {
        var targetIndex = isBookMode ? Math.floor(pageNumber / 2) : pageNumber
        if (targetIndex >= 0 && targetIndex < count) {
            positionViewAtIndex(targetIndex, ListView.Beginning)
        }
    }
    onCurrentIndexChanged: {
        if (isBookMode) {
            currentPage = currentIndex * 2
        } else {
            currentPage = currentIndex
        }
    }
    Behavior on contentX {
        enabled: isHorizontal
        SmoothedAnimation {
            duration: 200
            easing.type: Easing.OutCubic
        }
    }

    Behavior on contentY {
        enabled: !isHorizontal
        SmoothedAnimation {
            duration: scrollDuration
            easing.type: Easing.OutCubic
        }
    }
    MouseArea {
           anchors.fill: parent
           enabled: isHorizontal
           z: 1000
           preventStealing: true
           propagateComposedEvents: false

           onPressed: {
               dragStartX = mouseX
               isDragging = true
               mouse.accepted = true
           }

           onPositionChanged: {
               if (!isDragging || !isHorizontal) return
               mouse.accepted = true

               var delta = mouseX - dragStartX
               var newX = pagesView.contentX - delta

               // Constrain movemen
               if (newX < 0 || newX > (count - 1) * width) {
                   newX = pagesView.contentX - (delta * 0.3)
               }

               pagesView.contentX = newX
           }

           onReleased: {
               if (!isDragging || !isHorizontal) return
               isDragging = false
               mouse.accepted = true

               var delta = mouseX - dragStartX
               var targetPage = currentIndex

               if (Math.abs(delta) > dragThreshold) {
                   if (delta > 0 && currentIndex > 0) {
                       targetPage = currentIndex - 1
                   } else if (delta < 0 && currentIndex < count - 1) {
                       targetPage = currentIndex + 1
                   }
               }

               contentX = targetPage * width
               currentIndex = targetPage
           }
       }
}
