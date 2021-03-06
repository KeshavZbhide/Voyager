{
	'variables': {
	   'chromium_code': 1,
	   'use_v8_in_net': 1,
	},
	'targets': [
		{
			'target_name': 'KaminoContentShell',
			'type': 'executable',
			'include_dirs': [
        		'.',
        		'../third_party/WebKit/Source/WebKit/chromium/public',
        	],
			'dependencies': [
				'../content/content.gyp:content_app_both',
        		'../content/content.gyp:content_browser',
        		'../content/content.gyp:content_common',
        		'../content/content.gyp:content_gpu',
        		'../content/content.gyp:content_plugin',
        		'../content/content.gyp:content_ppapi_plugin',
        		'../content/content.gyp:content_renderer',
        		'../content/content.gyp:content_utility',
        		'../content/content.gyp:content_worker',
                '../content/content.gyp:content_shell_resources',
        		'../content/content_resources.gyp:content_resources',
        		'../base/base.gyp:base',
        		'../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
                '../sandbox/sandbox.gyp:sandbox',
        		'../url/url.gyp:url_lib',
        		'../ipc/ipc.gyp:ipc',
        		'../media/media.gyp:media',
        		'../net/net.gyp:net',
        		'../skia/skia.gyp:skia',
        		'../third_party/libxml/libxml.gyp:libxml',
                '../third_party/libpng/libpng.gyp:libpng',
        		'../third_party/zlib/zlib.gyp:minizip',
        		'../ui/gl/gl.gyp:gl',
        		'../ui/ui.gyp:ui',
        		'../v8/tools/gyp/v8.gyp:v8',
                '../third_party/WebKit/public/blink_test_runner.gyp:blink_test_runner',
                '../webkit/common/webkit_common.gyp:webkit_common',
                '../webkit/webkit_resources.gyp:webkit_resources',
                '../webkit/common/user_agent/webkit_user_agent.gyp:user_agent',
                '../webkit/support/webkit_support.gyp:glue',
      		],
      		'sources': [
        		'../content/browser/net/sqlite_persistent_cookie_store.cc',
        		'../content/browser/net/sqlite_persistent_cookie_store.h',
                'BasicUIMain.cpp',
                'KaminoAfterSetupSystemInit.cpp',
                'KaminoAfterSetupSystemInit.h',
                'KaminoBrowserContext.cpp',
                'KaminoBrowserContext.h',
                'KaminoBrowserMainParts.cpp',
                'KaminoBrowserMainParts.h',
                'KaminoContentBrowserClient.cpp',
                'KaminoContentBrowserClient.h',
				'KaminoContentRenderClient.h',
				'KaminoContentRenderClient.cpp',
                'KaminoDirectResource.cpp',
                'KaminoDirectResource.h',
                'KaminoDownloadManagerDelegate.cpp',
                'KaminoDownloadManagerDelegate.h',
                'KaminoFindWindow.cpp',
                'KaminoFindWindow.h',
                'KaminoGlobal.cpp',
                'KaminoGlobal.h',
                'KaminoGlowWindow.cpp',
                'KaminoGlowWindow.h',
                'KaminoHistoryModule.cpp',
                'KaminoHistoryModule.h',
                'KaminoIPCMessages.h',
                'KaminoLayeredToolTip.cpp',
                'KaminoLayeredToolTip.h',
                'KaminoMainDelegate.cpp',
                'KaminoMainDelegate.h',
                'KaminoMsgGenrator.cpp',
                'KaminoNetworkDelegate.cpp',
                'KaminoNetworkDelegate.h',
                'KaminoRenderViewHost.cpp',
                'KaminoRenderViewHost.h',
                'KaminoSilentFacebookPost.cpp',
                'KaminoSilentFacebookPost.h',
                'KaminoSimpleTimer.cpp',
                'KaminoSimpleTimer.h',
                'KaminoUpdateProcess.cpp',
                'KaminoUpdateProcess.h',
                'KaminoUpdateStatus.cpp',
                'KaminoUpdateStatus.h',
                'KaminoURLToolTip.cpp',
                'KaminoURLToolTip.h',
                'KaminoWaitWindow.cpp',
                'KaminoWaitWindow.h',
                'KaminoWebContentViewDelegate.cpp',
                'KaminoWebContentViewDelegate.h',
                'KButton.cpp',
                'KButton.h',
                'KTab.cpp',
                'KTab.h',
                'main.cc',
                'resource.h',
                'resource.rc',
                'UIMain.cpp',
                'UIMain.h',
      		],
            'msvs_settings': {
                'VCLinkerTool': {
                    # Set /SUBSYSTEM:WINDOWS.
                    'SubSystem': '2',
                    'EntryPointSymbol' : 'wWinMainCRTStartup',
                },
            },
            'conditions': [
                ['OS=="win" and win_use_allocator_shim==1', {
                    'dependencies': [
                        '../base/allocator/allocator.gyp:allocator',
                    ],
                }],
                ['OS=="win"', {
                    'resource_include_dirs': [
                        '<(SHARED_INTERMEDIATE_DIR)/webkit',
                    ],
                    'dependencies': [
                        '<(DEPTH)/webkit/webkit_resources.gyp:webkit_resources',
                        '<(DEPTH)/webkit/webkit_resources.gyp:webkit_strings',
                    ],
                    'configurations': {
                        'Debug_Base': {
                            'msvs_settings': {
                                'VCLinkerTool': {
                                    'LinkIncremental': '<(msvs_large_module_debug_link_mode)',
                                },
                            },
                        },
                    },
                }],
            ],     
		},
	],
}