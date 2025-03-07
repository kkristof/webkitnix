# -------------------------------------------------------------------
# Target file for the WebKit2 static library
#
# See 'Tools/qmake/README' for an overview of the build system
# -------------------------------------------------------------------

TEMPLATE = lib
TARGET = WebKit2

include(WebKit2.pri)

WEBKIT += wtf javascriptcore webcore

CONFIG += staticlib

RESOURCES += $$PWD/WebKit2.qrc

HEADERS += \
    Platform/CoreIPC/ArgumentDecoder.h \
    Platform/CoreIPC/ArgumentEncoder.h \
    Platform/CoreIPC/Arguments.h \
    Platform/CoreIPC/Attachment.h \
    Platform/CoreIPC/BinarySemaphore.h \
    Platform/CoreIPC/Connection.h \
    Platform/CoreIPC/DataReference.h \
    Platform/CoreIPC/HandleMessage.h \
    Platform/CoreIPC/MessageDecoder.h \
    Platform/CoreIPC/MessageEncoder.h \
    Platform/CoreIPC/MessageFlags.h \
    Platform/CoreIPC/MessageReceiver.h \
    Platform/CoreIPC/MessageReceiverMap.h \
    Platform/CoreIPC/MessageSender.h \
    Platform/CoreIPC/StringReference.h \
    Platform/Logging.h \
    Platform/Module.h \
    Platform/PlatformProcessIdentifier.h \
    Platform/SharedMemory.h \
    Platform/WorkQueue.h \
    PluginProcess/PluginControllerProxy.h \
    PluginProcess/PluginCreationParameters.h \
    PluginProcess/PluginProcess.h \
    PluginProcess/WebProcessConnection.h \
    Shared/API/c/WKArray.h \
    Shared/API/c/WKBase.h \
    Shared/API/c/WKCertificateInfo.h \
    Shared/API/c/WKConnectionRef.h \
    Shared/API/c/WKContextMenuItem.h \
    Shared/API/c/WKContextMenuItemTypes.h \
    Shared/API/c/WKData.h \
    Shared/API/c/WKDictionary.h \
    Shared/API/c/WKError.h \
    Shared/API/c/WKGeometry.h \
    Shared/API/c/WKGraphicsContext.h \
    Shared/API/c/WKImage.h \
    Shared/API/c/WKMutableArray.h \
    Shared/API/c/WKMutableDictionary.h \
    Shared/API/c/WKNumber.h \
    Shared/API/c/WKPageLoadTypes.h \
    Shared/API/c/WKPageVisibilityTypes.h \
    Shared/API/c/WKRenderLayer.h \
    Shared/API/c/WKRenderObject.h \
    Shared/API/c/WKSecurityOrigin.h \
    Shared/API/c/WKSerializedScriptValue.h \
    Shared/API/c/WKSharedAPICast.h \
    Shared/API/c/WKString.h \
    Shared/API/c/WKStringPrivate.h \
    Shared/API/c/WKType.h \
    Shared/API/c/WKURL.h \
    Shared/API/c/WKURLRequest.h \
    Shared/API/c/WKURLResponse.h \
    Shared/API/c/WKUserContentURLPattern.h \
    Shared/API/c/qt/WKImageQt.h \
    Shared/APIClientTraits.h \
    Shared/Authentication/AuthenticationManager.h \
    Shared/ShareableBitmap.h \
    Shared/CacheModel.h \
    Shared/ChildProcess.h \
    Shared/ChildProcessProxy.h \
    Shared/ConnectionStack.h \
    Shared/DictionaryPopupInfo.h \
    Shared/Downloads/Download.h \
    Shared/Downloads/DownloadManager.h \
    Shared/Downloads/qt/QtFileDownloader.h \
    Shared/EditorState.h \
    Shared/FontInfo.h \
    Shared/ImageOptions.h \
    Shared/ImmutableArray.h \
    Shared/ImmutableDictionary.h \
    Shared/LayerTreeContext.h \
    Shared/MutableArray.h \
    Shared/MutableDictionary.h \
    Shared/NativeWebKeyboardEvent.h \
    Shared/NativeWebMouseEvent.h \
    Shared/NativeWebWheelEvent.h \
    Shared/Network/NetworkProcessSupplement.h \
    Shared/OriginAndDatabases.h \
    Shared/PlatformPopupMenuData.h \
    Shared/PrintInfo.h \
    Shared/ProcessExecutablePath.h \
    Shared/SameDocumentNavigationType.h \
    Shared/SecurityOriginData.h \
    Shared/SessionState.h \
    Shared/StatisticsData.h \
    Shared/UpdateInfo.h \
    Shared/UserMessageCoders.h \
    Shared/VisitedLinkTable.h \
    Shared/WebCoreArgumentCoders.h \
    Shared/WebBackForwardListItem.h \
    Shared/WebBatteryStatus.h \
    Shared/WebCertificateInfo.h \
    Shared/WebConnection.h \
    Shared/WebConnectionClient.h \
    Shared/WebContextMenuItem.h \
    Shared/WebContextMenuItemData.h \
    Shared/WebError.h \
    Shared/WebEvent.h \
    Shared/WebEventConversion.h \
    Shared/WebFindOptions.h \
    Shared/WebGeolocationPosition.h \
    Shared/WebGeometry.h \
    Shared/WebGraphicsContext.h \
    Shared/WebHitTestResult.h \
    Shared/WebImage.h \
    Shared/WebNavigationDataStore.h \
    Shared/WebNumber.h \
    Shared/WebOpenPanelParameters.h \
    Shared/WebPageCreationParameters.h \
    Shared/WebPageGroupData.h \
    Shared/WebPopupItem.h \
    Shared/WebPreferencesStore.h \
    Shared/WebProcessCreationParameters.h \
    Shared/WebRenderLayer.h \
    Shared/WebRenderObject.h \
    Shared/WebURLRequest.h \
    Shared/WebURLResponse.h \
    Shared/WebUserContentURLPattern.h \
    Shared/CoordinatedGraphics/CoordinatedGraphicsArgumentCoders.h \
    Shared/CoordinatedGraphics/WebCoordinatedSurface.h \
    Shared/Plugins/Netscape/NetscapePluginModule.h \
    Shared/Plugins/NPRemoteObjectMap.h \
    Shared/Plugins/NPIdentifierData.h \
    Shared/Plugins/NPObjectMessageReceiver.h \
    Shared/Plugins/NPObjectProxy.h \
    Shared/Plugins/NPVariantData.h \
    Shared/Plugins/PluginModuleInfo.h \
    Shared/Plugins/PluginProcessCreationParameters.h \
    Shared/Plugins/PluginQuirks.h \
    Shared/qt/ArgumentCodersQt.h \
    Shared/qt/PlatformCertificateInfo.h \
    Shared/qt/WebEventFactoryQt.h \
    Shared/qt/QtNetworkReplyData.h \
    Shared/qt/QtNetworkRequestData.h \
    UIProcess/API/C/WKAPICast.h \
    UIProcess/API/C/WKAuthenticationChallenge.h \
    UIProcess/API/C/WKAuthenticationDecisionListener.h \
    UIProcess/API/C/WKBackForwardList.h \
    UIProcess/API/C/WKBackForwardListItem.h \
    UIProcess/API/C/WKBatteryManager.h \
    UIProcess/API/C/WKBatteryStatus.h \
    UIProcess/API/C/WKColorPickerResultListener.h \
    UIProcess/API/C/WKContext.h \
    UIProcess/API/C/WKContextPrivate.h \
    UIProcess/API/C/WKCredential.h \
    UIProcess/API/C/WKCredentialTypes.h \
    UIProcess/API/C/WKDatabaseManager.h \
    UIProcess/API/C/WKDownload.h \
    UIProcess/API/C/WKFrame.h \
    UIProcess/API/C/WKFramePolicyListener.h \
    UIProcess/API/C/WKGeolocationManager.h \
    UIProcess/API/C/WKGeolocationPermissionRequest.h \
    UIProcess/API/C/WKGeolocationPosition.h \
    UIProcess/API/C/WKGrammarDetail.h \
    UIProcess/API/C/WKHitTestResult.h \
    UIProcess/API/C/WKIconDatabase.h \
    UIProcess/API/C/WKInspector.h \
    UIProcess/API/C/WKOpenPanelParameters.h \
    UIProcess/API/C/WKOpenPanelResultListener.h \
    UIProcess/API/C/WKNavigationData.h \
    UIProcess/API/C/WKNetworkInfo.h \
    UIProcess/API/C/WKNetworkInfoManager.h \
    UIProcess/API/C/WKNotification.h \
    UIProcess/API/C/WKNotificationManager.h \
    UIProcess/API/C/WKNotificationPermissionRequest.h \
    UIProcess/API/C/WKNotificationProvider.h \
    UIProcess/API/C/WKPage.h \
    UIProcess/API/C/WKPageGroup.h \
    UIProcess/API/C/WKPagePrivate.h \
    UIProcess/API/C/WKPluginSiteDataManager.h \
    UIProcess/API/C/WKPreferences.h \
    UIProcess/API/C/WKPreferencesPrivate.h \
    UIProcess/API/C/WKProtectionSpace.h \
    UIProcess/API/C/WKProtectionSpaceTypes.h \
    UIProcess/API/C/WKTextChecker.h \
    UIProcess/API/C/WKVibration.h \
    UIProcess/API/C/WebKit2_C.h \
    UIProcess/API/C/qt/WKNativeEvent.h \
    UIProcess/API/C/qt/WKIconDatabaseQt.h \
    UIProcess/API/cpp/WKRetainPtr.h \
    UIProcess/API/cpp/qt/WKStringQt.h \
    UIProcess/API/cpp/qt/WKURLQt.h \
    UIProcess/API/qt/raw/qrawwebview_p.h \
    UIProcess/API/qt/raw/qrawwebview_p_p.h \
    UIProcess/Authentication/AuthenticationChallengeProxy.h \
    UIProcess/Authentication/AuthenticationDecisionListener.h \
    UIProcess/Authentication/WebCredential.h \
    UIProcess/Authentication/WebProtectionSpace.h \
    UIProcess/BackingStore.h \
    UIProcess/Downloads/DownloadProxy.h \
    UIProcess/CoordinatedGraphics/CoordinatedLayerTreeHostProxy.h \
    UIProcess/DefaultUndoController.h \
    UIProcess/DrawingAreaProxy.h \
    UIProcess/DrawingAreaProxyImpl.h \
    UIProcess/FindIndicator.h \
    UIProcess/GenericCallback.h \
    UIProcess/GeolocationPermissionRequestManagerProxy.h \
    UIProcess/GeolocationPermissionRequestProxy.h \
    UIProcess/Launcher/ProcessLauncher.h \
    UIProcess/Notifications/NotificationPermissionRequest.h \
    UIProcess/Notifications/NotificationPermissionRequestManagerProxy.h \
    UIProcess/Notifications/WebNotification.h \
    UIProcess/Notifications/WebNotificationManagerProxy.h \
    UIProcess/Notifications/WebNotificationProvider.h \
    UIProcess/PageClient.h \
    UIProcess/PageViewportController.h \
    UIProcess/PageViewportControllerClient.h \
    UIProcess/Plugins/PlugInAutoStartProvider.h \
    UIProcess/Plugins/PluginInfoStore.h \
    UIProcess/Plugins/PluginProcessProxy.h \
    UIProcess/Plugins/PluginProcessManager.h \
    UIProcess/ProcessModel.h \
    UIProcess/ResponsivenessTimer.h \
    UIProcess/StatisticsRequest.h \
    UIProcess/Storage/StorageManager.h \
    UIProcess/TextChecker.h \
    UIProcess/TextCheckerCompletion.h \
    UIProcess/VisitedLinkProvider.h \
    UIProcess/WebApplicationCacheManagerProxy.h \
    UIProcess/WebBackForwardList.h \
    UIProcess/WebBatteryManagerProxy.h \
    UIProcess/WebBatteryProvider.h \
    UIProcess/WebColorChooserProxy.h \
    UIProcess/WebColorPickerResultListenerProxy.h \
    UIProcess/WebConnectionToWebProcess.h \
    UIProcess/WebContext.h \
    UIProcess/WebContextClient.h \
    UIProcess/WebContextConnectionClient.h \
    UIProcess/WebContextInjectedBundleClient.h \
    UIProcess/WebContextMenuProxy.h \
    UIProcess/WebContextUserMessageCoders.h \
    UIProcess/WebCookieManagerProxy.h \
    UIProcess/WebCookieManagerProxyClient.h \
    UIProcess/WebDatabaseManagerProxy.h \
    UIProcess/WebDatabaseManagerProxyClient.h \
    UIProcess/WebDownloadClient.h \
    UIProcess/WebEditCommandProxy.h \
    UIProcess/WebFindClient.h \
    UIProcess/WebFormClient.h \
    UIProcess/WebFormSubmissionListenerProxy.h \
    UIProcess/WebFrameListenerProxy.h \
    UIProcess/WebFramePolicyListenerProxy.h \
    UIProcess/WebFrameProxy.h \
    UIProcess/WebFullScreenManagerProxy.h \
    UIProcess/WebGeolocationManagerProxy.h \
    UIProcess/WebGeolocationProvider.h \
    UIProcess/WebGrammarDetail.h \
    UIProcess/WebHistoryClient.h \
    UIProcess/WebIconDatabase.h \
    UIProcess/WebIconDatabaseClient.h \
    UIProcess/WebInspectorProxy.h \
    UIProcess/WebKeyValueStorageManagerProxy.h \
    UIProcess/WebLoaderClient.h \
    UIProcess/WebMediaCacheManagerProxy.h \
    UIProcess/WebNavigationData.h \
    UIProcess/WebOpenPanelResultListenerProxy.h \
    UIProcess/WebPageContextMenuClient.h \
    UIProcess/WebPageGroup.h \
    UIProcess/WebPageProxy.h \
    UIProcess/WebPolicyClient.h \
    UIProcess/WebPreferences.h \
    UIProcess/WebProcessProxy.h \
    UIProcess/WebResourceCacheManagerProxy.h \
    UIProcess/WebTextChecker.h \
    UIProcess/WebTextCheckerClient.h \
    UIProcess/WebUIClient.h \
    UIProcess/WebVibrationProvider.h \
    UIProcess/WebVibrationProxy.h \
    WebProcess/ApplicationCache/WebApplicationCacheManager.h \
    WebProcess/Battery/WebBatteryManager.h \
    WebProcess/Cookies/WebCookieManager.h \
    WebProcess/FullScreen/WebFullScreenManager.h \
    WebProcess/Geolocation/GeolocationPermissionRequestManager.h \
    WebProcess/Geolocation/WebGeolocationManager.h \
    WebProcess/IconDatabase/WebIconDatabaseProxy.h \
    WebProcess/InjectedBundle/API/c/WKBundleBackForwardList.h \
    WebProcess/InjectedBundle/API/c/WKBundleBackForwardListItem.h \
    WebProcess/InjectedBundle/API/c/WKBundleHitTestResult.h \
    WebProcess/InjectedBundle/API/c/WKBundleNavigationAction.h \
    WebProcess/InjectedBundle/API/c/WKBundleNodeHandle.h \
    WebProcess/InjectedBundle/API/c/WKBundleNodeHandlePrivate.h \
    WebProcess/InjectedBundle/API/c/WKBundlePage.h \
    WebProcess/InjectedBundle/API/c/WKBundlePageGroup.h \
    WebProcess/InjectedBundle/API/c/WKBundlePageOverlay.h \
    WebProcess/InjectedBundle/DOM/InjectedBundleNodeHandle.h \
    WebProcess/InjectedBundle/DOM/InjectedBundleRangeHandle.h \
    WebProcess/InjectedBundle/InjectedBundle.h \
    WebProcess/InjectedBundle/InjectedBundleClient.h \
    WebProcess/InjectedBundle/InjectedBundleDOMWindowExtension.h \
    WebProcess/InjectedBundle/InjectedBundleHitTestResult.h \
    WebProcess/InjectedBundle/InjectedBundleNavigationAction.h \
    WebProcess/InjectedBundle/InjectedBundlePageContextMenuClient.h \
    WebProcess/InjectedBundle/InjectedBundlePageDiagnosticLoggingClient.h \
    WebProcess/InjectedBundle/InjectedBundlePageFormClient.h \
    WebProcess/InjectedBundle/InjectedBundlePageFullScreenClient.h \
    WebProcess/InjectedBundle/InjectedBundlePagePolicyClient.h \
    WebProcess/InjectedBundle/InjectedBundlePageUIClient.h \
    WebProcess/InjectedBundle/InjectedBundleScriptWorld.h \
    WebProcess/InjectedBundle/InjectedBundleUserMessageCoders.h \
    WebProcess/MediaCache/WebMediaCacheManager.h \
    WebProcess/NetworkInfo/WebNetworkInfoManager.h \
    WebProcess/Notifications/NotificationPermissionRequestManager.h \
    WebProcess/Notifications/WebNotificationManager.h \
    WebProcess/ResourceCache/WebResourceCacheManager.h \
    WebProcess/Plugins/Netscape/JSNPMethod.h \
    WebProcess/Plugins/Netscape/JSNPObject.h \
    WebProcess/Plugins/Netscape/NPJSObject.h \
    WebProcess/Plugins/Netscape/NPRuntimeObjectMap.h \
    WebProcess/Plugins/Netscape/NPRuntimeUtilities.h \
    WebProcess/Plugins/Netscape/NetscapeBrowserFuncs.h \
    WebProcess/Plugins/Netscape/NetscapePlugin.h \
    WebProcess/Plugins/Netscape/NetscapePluginStream.h \
    WebProcess/Plugins/Plugin.h \
    WebProcess/Plugins/PluginController.h \
    WebProcess/Plugins/PluginView.h \
    WebProcess/Plugins/PluginProxy.h \
    WebProcess/Plugins/PluginProcessConnection.h \
    WebProcess/Plugins/PluginProcessConnectionManager.h \
    WebProcess/Storage/WebKeyValueStorageManager.h \
    WebProcess/WebCoreSupport/WebBatteryClient.h \
    WebProcess/WebCoreSupport/WebChromeClient.h \
    WebProcess/WebCoreSupport/WebColorChooser.h \
    WebProcess/WebCoreSupport/WebContextMenuClient.h \
    WebProcess/WebCoreSupport/WebDatabaseManager.h \
    WebProcess/WebCoreSupport/WebDragClient.h \
    WebProcess/WebCoreSupport/WebEditorClient.h \
    WebProcess/WebCoreSupport/WebErrors.h \
    WebProcess/WebCoreSupport/WebFrameLoaderClient.h \
    WebProcess/WebCoreSupport/WebGeolocationClient.h \
    WebProcess/WebCoreSupport/WebInspectorClient.h \
    WebProcess/WebCoreSupport/WebInspectorFrontendClient.h \
    WebProcess/WebCoreSupport/WebNetworkInfoClient.h \
    WebProcess/WebCoreSupport/WebNotificationClient.h \
    WebProcess/WebCoreSupport/WebPlatformStrategies.h \
    WebProcess/WebCoreSupport/WebPlugInClient.h \
    WebProcess/WebCoreSupport/WebPopupMenu.h \
    WebProcess/WebCoreSupport/WebSearchPopupMenu.h \
    WebProcess/WebCoreSupport/WebVibrationClient.h \
    WebProcess/WebCoreSupport/qt/WebFrameNetworkingContext.h \
    WebProcess/WebPage/DrawingArea.h \
    WebProcess/WebPage/DrawingAreaImpl.h \
    WebProcess/WebPage/EventDispatcher.h \
    WebProcess/WebPage/FindController.h \
    WebProcess/WebPage/CoordinatedGraphics/CoordinatedLayerTreeHost.h \
    WebProcess/WebPage/TapHighlightController.h \
    WebProcess/WebPage/PageOverlay.h \
    WebProcess/WebPage/WebContextMenu.h \
    WebProcess/WebPage/WebFrame.h \
    WebProcess/WebPage/WebInspector.h \
    WebProcess/WebPage/WebOpenPanelResultListener.h \
    WebProcess/WebPage/WebPage.h \
    WebProcess/WebPage/WebPageGroupProxy.h \
    WebProcess/WebPage/WebUndoStep.h \
    WebProcess/WebConnectionToUIProcess.h \
    WebProcess/WebProcessSupplement.h \
    WebProcess/WebProcess.h \
    WebProcess/qt/QtBuiltinBundle.h \
    WebProcess/qt/QtBuiltinBundlePage.h \
    WebProcess/qt/QtNetworkAccessManager.h \
    WebProcess/qt/QtNetworkReply.h

SOURCES += \
    Platform/CoreIPC/ArgumentCoders.cpp \
    Platform/CoreIPC/ArgumentDecoder.cpp \
    Platform/CoreIPC/ArgumentEncoder.cpp \
    Platform/CoreIPC/Attachment.cpp \
    Platform/CoreIPC/Connection.cpp \
    Platform/CoreIPC/DataReference.cpp \
    Platform/CoreIPC/MessageDecoder.cpp \
    Platform/CoreIPC/MessageEncoder.cpp \
    Platform/CoreIPC/MessageReceiverMap.cpp \
    Platform/CoreIPC/StringReference.cpp \
    Platform/Logging.cpp \
    Platform/Module.cpp \
    Platform/WorkQueue.cpp \
    Platform/qt/LoggingQt.cpp \
    Platform/qt/ModuleQt.cpp \
    PluginProcess/PluginControllerProxy.cpp \
    PluginProcess/PluginCreationParameters.cpp \
    PluginProcess/PluginProcess.cpp \
    PluginProcess/WebProcessConnection.cpp \
    PluginProcess/qt/PluginControllerProxyQt.cpp \
    PluginProcess/qt/PluginProcessMainQt.cpp \
    PluginProcess/qt/PluginProcessQt.cpp \
    Shared/API/c/WKArray.cpp \
    Shared/API/c/WKCertificateInfo.cpp \
    Shared/API/c/WKConnectionRef.cpp \
    Shared/API/c/WKContextMenuItem.cpp \
    Shared/API/c/WKData.cpp \
    Shared/API/c/WKDictionary.cpp \
    Shared/API/c/WKError.cpp \
    Shared/API/c/WKGeometry.cpp \
    Shared/API/c/WKGraphicsContext.cpp \
    Shared/API/c/WKImage.cpp \
    Shared/API/c/WKMutableArray.cpp \
    Shared/API/c/WKMutableDictionary.cpp \
    Shared/API/c/WKNumber.cpp \
    Shared/API/c/WKRenderLayer.cpp \
    Shared/API/c/WKRenderObject.cpp \
    Shared/API/c/WKSecurityOrigin.cpp \
    Shared/API/c/WKSerializedScriptValue.cpp \
    Shared/API/c/WKString.cpp \
    Shared/API/c/WKType.cpp \
    Shared/API/c/WKURL.cpp \
    Shared/API/c/WKURLRequest.cpp \
    Shared/API/c/WKURLResponse.cpp \
    Shared/API/c/WKUserContentURLPattern.cpp \
    Shared/API/c/qt/WKImageQt.cpp \
    Shared/APIClientTraits.cpp \
    Shared/APIObject.cpp \
    Shared/Authentication/AuthenticationManager.cpp \
    Shared/Plugins/Netscape/NetscapePluginModule.cpp \
    Shared/Plugins/Netscape/NetscapePluginModuleNone.cpp \
    Shared/Plugins/Netscape/x11/NetscapePluginModuleX11.cpp \
    Shared/CacheModel.cpp \
    Shared/ShareableBitmap.cpp \
    Shared/Plugins/NPRemoteObjectMap.cpp \
    Shared/Plugins/NPIdentifierData.cpp \
    Shared/Plugins/NPObjectMessageReceiver.cpp \
    Shared/Plugins/NPObjectProxy.cpp \
    Shared/Plugins/NPVariantData.cpp \
    Shared/Plugins/PluginModuleInfo.cpp \
    Shared/Plugins/PluginProcessCreationParameters.cpp \
    Shared/ChildProcess.cpp \
    Shared/ChildProcessProxy.cpp \
    Shared/ConnectionStack.cpp \
    Shared/DictionaryPopupInfo.cpp \
    Shared/Downloads/Download.cpp \
    Shared/Downloads/DownloadManager.cpp \
    Shared/Downloads/qt/DownloadQt.cpp \
    Shared/Downloads/qt/QtFileDownloader.cpp \
    Shared/EditorState.cpp \
    Shared/FontInfo.cpp \
    Shared/ImmutableArray.cpp \
    Shared/ImmutableDictionary.cpp \
    Shared/MutableArray.cpp \
    Shared/MutableDictionary.cpp \
    Shared/OriginAndDatabases.cpp \
    Shared/PlatformPopupMenuData.cpp \
    Shared/PrintInfo.cpp \
    Shared/SecurityOriginData.cpp \
    Shared/SessionState.cpp \
    Shared/StatisticsData.cpp \
    Shared/UpdateInfo.cpp \
    Shared/VisitedLinkTable.cpp \
    Shared/WebBackForwardListItem.cpp \
    Shared/WebBatteryStatus.cpp \
    Shared/WebConnection.cpp \
    Shared/WebConnectionClient.cpp \
    Shared/WebContextMenuItem.cpp \
    Shared/WebContextMenuItemData.cpp \
    Shared/WebCoreArgumentCoders.cpp \
    Shared/WebError.cpp \
    Shared/WebEvent.cpp \
    Shared/WebEventConversion.cpp \
    Shared/WebGestureEvent.cpp \
    Shared/WebGeolocationPosition.cpp \
    Shared/WebGraphicsContext.cpp \
    Shared/WebHitTestResult.cpp \
    Shared/WebKeyboardEvent.cpp \
    Shared/WebKit2Initialize.cpp \
    Shared/WebImage.cpp \
    Shared/WebMouseEvent.cpp \
    Shared/WebNetworkInfo.cpp \
    Shared/WebOpenPanelParameters.cpp \
    Shared/WebPageCreationParameters.cpp \
    Shared/WebPageGroupData.cpp \
    Shared/WebPlatformTouchPoint.cpp \
    Shared/WebPopupItem.cpp \
    Shared/WebPreferencesStore.cpp \
    Shared/WebProcessCreationParameters.cpp \
    Shared/WebRenderLayer.cpp \
    Shared/WebRenderObject.cpp \
    Shared/WebTouchEvent.cpp \
    Shared/WebURLRequest.cpp \
    Shared/WebURLResponse.cpp \
    Shared/WebWheelEvent.cpp \
    Shared/CoordinatedGraphics/CoordinatedGraphicsArgumentCoders.cpp \
    Shared/CoordinatedGraphics/WebCoordinatedSurface.cpp \
    Shared/CoordinatedGraphics/WebCustomFilterProgramProxy.cpp \
    Shared/qt/ArgumentCodersQt.cpp \
    Shared/qt/LayerTreeContextQt.cpp \
    Shared/qt/ShareableBitmapQt.cpp \
    Shared/qt/NativeWebKeyboardEventQt.cpp \
    Shared/qt/NativeWebMouseEventQt.cpp \
    Shared/qt/NativeWebWheelEventQt.cpp \
    Shared/qt/ProcessExecutablePathQt.cpp \
    Shared/qt/WebCoreArgumentCodersQt.cpp \
    Shared/qt/WebEventFactoryQt.cpp \
    Shared/qt/QtNetworkReplyData.cpp \
    Shared/qt/QtNetworkRequestData.cpp \
    Shared/qt/WebURLRequestQt.cpp \
    Shared/qt/WebURLResponseQt.cpp \
    UIProcess/API/C/WKAuthenticationChallenge.cpp \
    UIProcess/API/C/WKAuthenticationDecisionListener.cpp \
    UIProcess/API/C/WKBackForwardList.cpp \
    UIProcess/API/C/WKBackForwardListItem.cpp \
    UIProcess/API/C/WKBatteryManager.cpp \
    UIProcess/API/C/WKBatteryStatus.cpp \
    UIProcess/API/C/WKColorPickerResultListener.cpp \
    UIProcess/API/C/WKContext.cpp \
    UIProcess/API/C/WKCredential.cpp \
    UIProcess/API/C/WKDatabaseManager.cpp \
    UIProcess/API/C/WKDownload.cpp \
    UIProcess/API/C/WKFrame.cpp \
    UIProcess/API/C/WKFramePolicyListener.cpp \
    UIProcess/API/C/WKGeolocationManager.cpp \
    UIProcess/API/C/WKGeolocationPermissionRequest.cpp \
    UIProcess/API/C/WKGeolocationPosition.cpp \
    UIProcess/API/C/WKGrammarDetail.cpp \
    UIProcess/API/C/WKHitTestResult.cpp \
    UIProcess/API/C/WKIconDatabase.cpp \
    UIProcess/API/C/WKInspector.cpp \
    UIProcess/API/C/WKNotification.cpp \
    UIProcess/API/C/WKNotificationManager.cpp \
    UIProcess/API/C/WKNotificationPermissionRequest.cpp \
    UIProcess/API/C/WKOpenPanelParameters.cpp \
    UIProcess/API/C/WKOpenPanelResultListener.cpp \
    UIProcess/API/C/WKNavigationData.cpp \
    UIProcess/API/C/WKNetworkInfo.cpp \
    UIProcess/API/C/WKNetworkInfoManager.cpp \
    UIProcess/API/C/WKPage.cpp \
    UIProcess/API/C/WKPageGroup.cpp \
    UIProcess/API/C/WKPluginSiteDataManager.cpp \
    UIProcess/API/C/WKPreferences.cpp \
    UIProcess/API/C/WKProtectionSpace.cpp \
    UIProcess/API/C/WKResourceCacheManager.cpp \
    UIProcess/API/C/WKTextChecker.cpp \
    UIProcess/API/C/WKVibration.cpp \
    UIProcess/API/C/qt/WKIconDatabaseQt.cpp \
    UIProcess/API/cpp/qt/WKStringQt.cpp \
    UIProcess/API/cpp/qt/WKURLQt.cpp \
    UIProcess/API/qt/raw/qrawwebview.cpp \
    UIProcess/Authentication/AuthenticationChallengeProxy.cpp \
    UIProcess/Authentication/AuthenticationDecisionListener.cpp \
    UIProcess/Authentication/WebCredential.cpp \
    UIProcess/Authentication/WebProtectionSpace.cpp \
    UIProcess/BackingStore.cpp \
    UIProcess/qt/BackingStoreQt.cpp \
    UIProcess/CoordinatedGraphics/CoordinatedLayerTreeHostProxy.cpp \
    UIProcess/DefaultUndoController.cpp \
    UIProcess/Downloads/DownloadProxy.cpp \
    UIProcess/Downloads/DownloadProxyMap.cpp \
    UIProcess/DrawingAreaProxy.cpp \
    UIProcess/DrawingAreaProxyImpl.cpp \
    UIProcess/FindIndicator.cpp \
    UIProcess/GeolocationPermissionRequestManagerProxy.cpp \
    UIProcess/GeolocationPermissionRequestProxy.cpp \
    UIProcess/TextCheckerCompletion.cpp \
    UIProcess/Launcher/ProcessLauncher.cpp \
    UIProcess/Launcher/qt/ProcessLauncherQt.cpp \
    UIProcess/Notifications/NotificationPermissionRequest.cpp \
    UIProcess/Notifications/NotificationPermissionRequestManagerProxy.cpp \
    UIProcess/Notifications/WebNotification.cpp \
    UIProcess/Notifications/WebNotificationManagerProxy.cpp \
    UIProcess/Notifications/WebNotificationProvider.cpp \
    UIProcess/PageViewportController.cpp \
    UIProcess/Plugins/PlugInAutoStartProvider.cpp \
    UIProcess/Plugins/PluginInfoStore.cpp \
    UIProcess/Plugins/PluginProcessProxy.cpp \
    UIProcess/Plugins/PluginProcessManager.cpp \
    UIProcess/Plugins/WebPluginSiteDataManager.cpp \
    UIProcess/Plugins/qt/PluginProcessProxyQt.cpp \
    UIProcess/Plugins/unix/PluginInfoStoreUnix.cpp \
    UIProcess/ResponsivenessTimer.cpp \
    UIProcess/StatisticsRequest.cpp \
    UIProcess/Storage/StorageManager.cpp \
    UIProcess/VisitedLinkProvider.cpp \
    UIProcess/WebApplicationCacheManagerProxy.cpp \
    UIProcess/WebBackForwardList.cpp \
    UIProcess/WebBatteryManagerProxy.cpp \
    UIProcess/WebBatteryProvider.cpp \
    UIProcess/WebColorChooserProxy.cpp \
    UIProcess/WebColorPickerResultListenerProxy.cpp \
    UIProcess/WebConnectionToWebProcess.cpp \
    UIProcess/WebContext.cpp \
    UIProcess/WebContextClient.cpp \
    UIProcess/WebContextConnectionClient.cpp \
    UIProcess/WebContextInjectedBundleClient.cpp \
    UIProcess/WebContextMenuProxy.cpp \
    UIProcess/WebCookieManagerProxy.cpp \
    UIProcess/WebCookieManagerProxyClient.cpp \
    UIProcess/WebDatabaseManagerProxy.cpp \
    UIProcess/WebDatabaseManagerProxyClient.cpp \
    UIProcess/WebDownloadClient.cpp \
    UIProcess/WebEditCommandProxy.cpp \
    UIProcess/WebFindClient.cpp \
    UIProcess/WebFormClient.cpp \
    UIProcess/WebFormSubmissionListenerProxy.cpp \
    UIProcess/WebFrameListenerProxy.cpp \
    UIProcess/WebFramePolicyListenerProxy.cpp \
    UIProcess/WebFrameProxy.cpp \
    UIProcess/WebFullScreenManagerProxy.cpp \
    UIProcess/WebGeolocationManagerProxy.cpp \
    UIProcess/WebGeolocationProvider.cpp \
    UIProcess/WebGrammarDetail.cpp \
    UIProcess/WebHistoryClient.cpp \
    UIProcess/WebIconDatabase.cpp \
    UIProcess/WebIconDatabaseClient.cpp \
    UIProcess/WebInspectorProxy.cpp \
    UIProcess/WebKeyValueStorageManagerProxy.cpp \
    UIProcess/WebLoaderClient.cpp \
    UIProcess/WebMediaCacheManagerProxy.cpp \
    UIProcess/WebNavigationData.cpp \
    UIProcess/WebNetworkInfoManagerProxy.cpp \
    UIProcess/WebNetworkInfoProvider.cpp \
    UIProcess/WebOpenPanelResultListenerProxy.cpp \
    UIProcess/WebPageContextMenuClient.cpp \
    UIProcess/WebPageGroup.cpp \
    UIProcess/WebPageProxy.cpp \
    UIProcess/WebPolicyClient.cpp \
    UIProcess/WebPreferences.cpp \
    UIProcess/WebProcessProxy.cpp \
    UIProcess/WebResourceCacheManagerProxy.cpp \
    UIProcess/WebTextChecker.cpp \
    UIProcess/WebTextCheckerClient.cpp \
    UIProcess/WebUIClient.cpp \
    UIProcess/WebVibrationProvider.cpp \
    UIProcess/WebVibrationProxy.cpp \
    UIProcess/qt/TextCheckerQt.cpp \
    UIProcess/qt/WebContextQt.cpp \
    UIProcess/qt/WebFullScreenManagerProxyQt.cpp \
    UIProcess/qt/WebInspectorProxyQt.cpp \
    UIProcess/qt/WebPageProxyQt.cpp \
    UIProcess/qt/WebPreferencesQt.cpp \
    UIProcess/qt/WebProcessProxyQt.cpp \
    WebProcess/ApplicationCache/WebApplicationCacheManager.cpp \
    WebProcess/Battery/WebBatteryManager.cpp \
    WebProcess/Cookies/WebCookieManager.cpp \
    WebProcess/Cookies/qt/WebCookieManagerQt.cpp \
    WebProcess/FullScreen/WebFullScreenManager.cpp \
    WebProcess/Geolocation/GeolocationPermissionRequestManager.cpp \
    WebProcess/Geolocation/WebGeolocationManager.cpp \
    WebProcess/IconDatabase/WebIconDatabaseProxy.cpp \
    WebProcess/InjectedBundle/API/c/WKBundle.cpp \
    WebProcess/InjectedBundle/API/c/WKBundleBackForwardList.cpp \
    WebProcess/InjectedBundle/API/c/WKBundleBackForwardListItem.cpp \
    WebProcess/InjectedBundle/API/c/WKBundleDOMWindowExtension.cpp \
    WebProcess/InjectedBundle/API/c/WKBundleFrame.cpp \
    WebProcess/InjectedBundle/API/c/WKBundleHitTestResult.cpp \
    WebProcess/InjectedBundle/API/c/WKBundleInspector.cpp \
    WebProcess/InjectedBundle/API/c/WKBundleNavigationAction.cpp \
    WebProcess/InjectedBundle/API/c/WKBundleNodeHandle.cpp \
    WebProcess/InjectedBundle/API/c/WKBundlePage.cpp \
    WebProcess/InjectedBundle/API/c/WKBundlePageGroup.cpp \
    WebProcess/InjectedBundle/API/c/WKBundlePageOverlay.cpp \
    WebProcess/InjectedBundle/API/c/WKBundleScriptWorld.cpp \
    WebProcess/InjectedBundle/DOM/InjectedBundleNodeHandle.cpp \
    WebProcess/InjectedBundle/DOM/InjectedBundleRangeHandle.cpp \
    WebProcess/InjectedBundle/InjectedBundle.cpp \
    WebProcess/InjectedBundle/InjectedBundleBackForwardList.cpp \
    WebProcess/InjectedBundle/InjectedBundleBackForwardListItem.cpp \
    WebProcess/InjectedBundle/InjectedBundleClient.cpp \
    WebProcess/InjectedBundle/InjectedBundleDOMWindowExtension.cpp \
    WebProcess/InjectedBundle/InjectedBundleHitTestResult.cpp \
    WebProcess/InjectedBundle/InjectedBundleNavigationAction.cpp \
    WebProcess/InjectedBundle/InjectedBundlePageContextMenuClient.cpp \
    WebProcess/InjectedBundle/InjectedBundlePageDiagnosticLoggingClient.cpp \    
    WebProcess/InjectedBundle/InjectedBundlePageEditorClient.cpp \
    WebProcess/InjectedBundle/InjectedBundlePageFormClient.cpp \
    WebProcess/InjectedBundle/InjectedBundlePageFullScreenClient.cpp \
    WebProcess/InjectedBundle/InjectedBundlePageLoaderClient.cpp \
    WebProcess/InjectedBundle/InjectedBundlePagePolicyClient.cpp \
    WebProcess/InjectedBundle/InjectedBundlePageResourceLoadClient.cpp \
    WebProcess/InjectedBundle/InjectedBundlePageUIClient.cpp \
    WebProcess/InjectedBundle/InjectedBundleScriptWorld.cpp \
    WebProcess/InjectedBundle/qt/InjectedBundleQt.cpp \
    WebProcess/MediaCache/WebMediaCacheManager.cpp \
    WebProcess/NetworkInfo/WebNetworkInfoManager.cpp \
    WebProcess/Notifications/NotificationPermissionRequestManager.cpp \
    WebProcess/Notifications/WebNotificationManager.cpp \
    WebProcess/ResourceCache/WebResourceCacheManager.cpp \
    WebProcess/Plugins/Netscape/JSNPMethod.cpp \
    WebProcess/Plugins/Netscape/JSNPObject.cpp \
    WebProcess/Plugins/Netscape/NPJSObject.cpp \
    WebProcess/Plugins/Netscape/NPRuntimeObjectMap.cpp \
    WebProcess/Plugins/Netscape/NPRuntimeUtilities.cpp \
    WebProcess/Plugins/Netscape/NetscapeBrowserFuncs.cpp \
    WebProcess/Plugins/Netscape/NetscapePlugin.cpp \
    WebProcess/Plugins/Netscape/NetscapePluginNone.cpp \
    WebProcess/Plugins/Netscape/NetscapePluginStream.cpp \
    WebProcess/Plugins/Netscape/x11/NetscapePluginX11.cpp \
    WebProcess/Plugins/Netscape/qt/PluginProxyQt.cpp \
    WebProcess/Plugins/Plugin.cpp \
    WebProcess/Plugins/PluginView.cpp \
    WebProcess/Plugins/PluginProxy.cpp \
    WebProcess/Plugins/PluginProcessConnection.cpp \
    WebProcess/Plugins/PluginProcessConnectionManager.cpp \
    WebProcess/Storage/StorageAreaImpl.cpp \
    WebProcess/Storage/StorageAreaMap.cpp \
    WebProcess/Storage/StorageNamespaceImpl.cpp \
    WebProcess/Storage/WebKeyValueStorageManager.cpp \
    WebProcess/WebCoreSupport/WebBatteryClient.cpp \
    WebProcess/WebCoreSupport/WebChromeClient.cpp \
    WebProcess/WebCoreSupport/WebColorChooser.cpp \
    WebProcess/WebCoreSupport/WebContextMenuClient.cpp \
    WebProcess/WebCoreSupport/WebDatabaseManager.cpp \
    WebProcess/WebCoreSupport/WebDragClient.cpp \
    WebProcess/WebCoreSupport/WebEditorClient.cpp \
    WebProcess/WebCoreSupport/WebFrameLoaderClient.cpp \
    WebProcess/WebCoreSupport/WebGeolocationClient.cpp \
    WebProcess/WebCoreSupport/WebInspectorClient.cpp \
    WebProcess/WebCoreSupport/WebInspectorFrontendClient.cpp \
    WebProcess/WebCoreSupport/WebNetworkInfoClient.cpp \
    WebProcess/WebCoreSupport/WebNotificationClient.cpp \
    WebProcess/WebCoreSupport/WebPlatformStrategies.cpp \
    WebProcess/WebCoreSupport/WebPlugInClient.cpp \
    WebProcess/WebCoreSupport/WebPopupMenu.cpp \
    WebProcess/WebCoreSupport/WebSearchPopupMenu.cpp \
    WebProcess/WebCoreSupport/WebVibrationClient.cpp \
    WebProcess/WebCoreSupport/qt/WebContextMenuClientQt.cpp \
    WebProcess/WebCoreSupport/qt/WebErrorsQt.cpp \
    WebProcess/WebCoreSupport/qt/WebDragClientQt.cpp \
    WebProcess/WebCoreSupport/qt/WebFrameNetworkingContext.cpp \
    WebProcess/WebCoreSupport/qt/WebPopupMenuQt.cpp \
    WebProcess/WebPage/DecoderAdapter.cpp \
    WebProcess/WebPage/DrawingArea.cpp \
    WebProcess/WebPage/DrawingAreaImpl.cpp \
    WebProcess/WebPage/EncoderAdapter.cpp \
    WebProcess/WebPage/EventDispatcher.cpp \
    WebProcess/WebPage/FindController.cpp \
    WebProcess/WebPage/CoordinatedGraphics/CoordinatedLayerTreeHost.cpp \
    WebProcess/WebPage/TapHighlightController.cpp \
    WebProcess/WebPage/LayerTreeHost.cpp \
    WebProcess/WebPage/PageOverlay.cpp \
    WebProcess/WebPage/WebBackForwardListProxy.cpp \
    WebProcess/WebPage/WebContextMenu.cpp \
    WebProcess/WebPage/WebFrame.cpp \
    WebProcess/WebPage/WebInspector.cpp \
    WebProcess/WebPage/WebOpenPanelResultListener.cpp \
    WebProcess/WebPage/WebPage.cpp \
    WebProcess/WebPage/WebPageGroupProxy.cpp \
    WebProcess/WebPage/WebUndoStep.cpp \
    WebProcess/WebPage/qt/WebInspectorQt.cpp \
    WebProcess/WebPage/qt/WebPageQt.cpp \
    WebProcess/WebConnectionToUIProcess.cpp \
    WebProcess/WebProcess.cpp \
    WebProcess/qt/QtBuiltinBundle.cpp \
    WebProcess/qt/QtBuiltinBundlePage.cpp \
    WebProcess/qt/QtNetworkAccessManager.cpp \
    WebProcess/qt/QtNetworkReply.cpp \
    WebProcess/qt/WebProcessMainQt.cpp \
    WebProcess/qt/WebProcessQt.cpp

have?(QTQUICK) {
    QT += qml quick quick-private

    HEADERS += \
        UIProcess/API/qt/qwebpreferences_p.h \
        UIProcess/API/qt/qwebpreferences_p_p.h \
        UIProcess/API/qt/qwebdownloaditem_p.h \
        UIProcess/API/qt/qwebdownloaditem_p_p.h \
        UIProcess/API/qt/qwebpermissionrequest_p.h \
        UIProcess/API/qt/qtwebsecurityorigin_p.h \
        UIProcess/API/qt/qwebloadrequest_p.h \
        UIProcess/API/qt/qwebnavigationrequest_p.h \
        UIProcess/API/qt/qquickwebpage_p.h \
        UIProcess/API/qt/qquickwebpage_p_p.h \
        UIProcess/API/qt/qquickwebview_p.h \
        UIProcess/API/qt/qquickwebview_p_p.h \
        UIProcess/API/qt/qquicknetworkreply_p.h \
        UIProcess/API/qt/qquicknetworkrequest_p.h \
        UIProcess/API/qt/qquickurlschemedelegate_p.h \
        UIProcess/API/qt/qwebkittest_p.h \
        UIProcess/qt/PageViewportControllerClientQt.h \
        UIProcess/qt/QtWebContext.h \
        UIProcess/qt/QtWebPageEventHandler.h \
        UIProcess/qt/QtGestureRecognizer.h \
        UIProcess/qt/QtPanGestureRecognizer.h \
        UIProcess/qt/QtPinchGestureRecognizer.h \
        UIProcess/qt/QtTapGestureRecognizer.h \
        UIProcess/qt/QtWebError.h \
        UIProcess/qt/QtDialogRunner.h \
        UIProcess/qt/QtDownloadManager.h \
        UIProcess/qt/QtPageClient.h \
        UIProcess/qt/QtWebPagePolicyClient.h \
        UIProcess/qt/QtWebPageSGNode.h \
        UIProcess/qt/QtWebPageUIClient.h \
        UIProcess/qt/QtWebIconDatabaseClient.h \
        UIProcess/qt/WebContextMenuProxyQt.h \
        UIProcess/qt/WebGeolocationProviderQt.h \
        UIProcess/qt/WebPopupMenuProxyQt.h

    SOURCES += \
        UIProcess/API/qt/qwebdownloaditem.cpp \
        UIProcess/API/qt/qwebpermissionrequest.cpp \
        UIProcess/API/qt/qtwebsecurityorigin.cpp \
        UIProcess/API/qt/qwebloadrequest.cpp \
        UIProcess/API/qt/qwebnavigationrequest.cpp \
        UIProcess/API/qt/qquickwebpage.cpp \
        UIProcess/API/qt/qquickwebview.cpp \
        UIProcess/API/qt/qwebiconimageprovider.cpp \
        UIProcess/API/qt/qquicknetworkreply.cpp \
        UIProcess/API/qt/qquicknetworkrequest.cpp \
        UIProcess/API/qt/qquickurlschemedelegate.cpp \
        UIProcess/API/qt/qwebpreferences.cpp \
        UIProcess/API/qt/qwebkittest.cpp \
        UIProcess/qt/PageViewportControllerClientQt.cpp \
        UIProcess/qt/QtDialogRunner.cpp \
        UIProcess/qt/QtDownloadManager.cpp \
        UIProcess/qt/QtPageClient.cpp \
        UIProcess/qt/QtWebPagePolicyClient.cpp \
        UIProcess/qt/QtWebPageSGNode.cpp \
        UIProcess/qt/QtWebPageEventHandler.cpp \
        UIProcess/qt/QtGestureRecognizer.cpp \
        UIProcess/qt/QtPanGestureRecognizer.cpp \
        UIProcess/qt/QtPinchGestureRecognizer.cpp \
        UIProcess/qt/QtTapGestureRecognizer.cpp \
        UIProcess/qt/QtWebContext.cpp \
        UIProcess/qt/QtWebError.cpp \
        UIProcess/qt/WebContextMenuProxyQt.cpp \
        UIProcess/qt/WebGeolocationProviderQt.cpp \
        UIProcess/qt/WebPopupMenuProxyQt.cpp \
        UIProcess/qt/QtWebIconDatabaseClient.cpp \
        UIProcess/qt/QtWebPageUIClient.cpp

    enable?(INPUT_TYPE_COLOR) {
        HEADERS += \
            UIProcess/qt/WebColorChooserProxyQt.h
        SOURCES += \
            UIProcess/qt/WebColorChooserProxyQt.cpp
    }
}

mac: {
    use?(QTKIT) {
        DEFINES += NSGEOMETRY_TYPES_SAME_AS_CGGEOMETRY_TYPES
        isEqual(QT_ARCH, "i386") {
            DEFINES+=NS_BUILD_32_LIKE_64
        }
        INCLUDEPATH += \
            $$PWD/../../WebKitLibraries/
        HEADERS += \
            WebProcess/WebCoreSupport/qt/WebSystemInterface.h
        OBJECTIVE_SOURCES += \
            WebProcess/WebCoreSupport/qt/WebSystemInterface.mm
    }
    INCLUDEPATH += \
        Platform/mac \
        Platform/CoreIPC/mac

    HEADERS += \
        Platform/mac/MachUtilities.h \
        Platform/CoreIPC/mac/MachPort.h

    SOURCES += \
        Platform/CoreIPC/mac/ConnectionMac.cpp \
        Platform/mac/MachUtilities.cpp \
        Platform/mac/WorkQueueMac.cpp \
        Platform/mac/SharedMemoryMac.cpp

} else:win32 {
    SOURCES += \
        Platform/CoreIPC/win/ConnectionWin.cpp \
        Platform/win/WorkQueueWin.cpp \
        Platform/win/SharedMemoryWin.cpp
} else {
    SOURCES += \
        Platform/CoreIPC/unix/AttachmentUnix.cpp \
        Platform/CoreIPC/unix/ConnectionUnix.cpp \
        Platform/qt/WorkQueueQt.cpp \
        Platform/unix/SharedMemoryUnix.cpp
}

win32 {
    SOURCES += \
        Platform/CoreIPC/win/BinarySemaphoreWin.cpp
} else {
    SOURCES += \
        Platform/CoreIPC/BinarySemaphore.cpp
}

enable?(SECCOMP_FILTERS) {
    HEADERS += \
        Shared/linux/SeccompFilters/OpenSyscall.h \
        Shared/linux/SeccompFilters/SeccompBroker.h \
        Shared/linux/SeccompFilters/SeccompFilters.h \
        Shared/linux/SeccompFilters/SigactionSyscall.h \
        Shared/linux/SeccompFilters/SigprocmaskSyscall.h \
        Shared/linux/SeccompFilters/Syscall.h \
        Shared/linux/SeccompFilters/SyscallPolicy.h \
        WebProcess/qt/SeccompFiltersWebProcessQt.h

    SOURCES += \
        Shared/linux/SeccompFilters/OpenSyscall.cpp \
        Shared/linux/SeccompFilters/SeccompBroker.cpp \
        Shared/linux/SeccompFilters/SeccompFilters.cpp \
        Shared/linux/SeccompFilters/SigactionSyscall.cpp \
        Shared/linux/SeccompFilters/SigprocmaskSyscall.cpp \
        Shared/linux/SeccompFilters/Syscall.cpp \
        Shared/linux/SeccompFilters/SyscallPolicy.cpp \
        WebProcess/qt/SeccompFiltersWebProcessQt.cpp

    DEFINES += SOURCE_DIR=\\\"$${ROOT_WEBKIT_DIR}\\\"
}

enable?(INSPECTOR_SERVER) {
    HEADERS += \
        UIProcess/InspectorServer/WebInspectorServer.h \
        UIProcess/InspectorServer/WebSocketServer.h \
        UIProcess/InspectorServer/WebSocketServerClient.h \
        UIProcess/InspectorServer/WebSocketServerConnection.h \
        UIProcess/InspectorServer/qt/WebSocketServerQt.h

    SOURCES += \
        UIProcess/InspectorServer/WebInspectorServer.cpp \
        UIProcess/InspectorServer/WebSocketServer.cpp \
        UIProcess/InspectorServer/WebSocketServerConnection.cpp \
        UIProcess/InspectorServer/qt/WebInspectorServerQt.cpp \
        UIProcess/InspectorServer/qt/WebSocketServerQt.cpp
}

enable?(TOUCH_EVENTS) {
    HEADERS += \
        Shared/NativeWebTouchEvent.h
    SOURCES += \
        Shared/qt/NativeWebTouchEventQt.cpp
}

enable?(GESTURE_EVENTS) {
    HEADERS += \
        Shared/NativeWebGestureEvent.h
    SOURCES += \
        Shared/qt/NativeWebGestureEventQt.cpp
}

have?(qtlocation):enable?(GEOLOCATION): QT += location

use?(3D_GRAPHICS): WEBKIT += angle

use?(PLUGIN_BACKEND_XLIB) {
    DEFINES += XP_UNIX
    PKGCONFIG += x11
    DEFINES += MOZ_X11
}

include(DerivedSources.pri)
