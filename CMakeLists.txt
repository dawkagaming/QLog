cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(qlog
    VERSION 0.51.0
    LANGUAGES CXX
)

add_compile_definitions(VERSION="${PROJECT_VERSION}")

find_package(Qt6 REQUIRED
    COMPONENTS
    Core
    Network
    Gui
    Widgets
    LinguistTools
    Sql
    Xml
    Charts
    WebEngineWidgets
    SerialPort
    QuickWidgets
    WebChannel
    WebSockets
    PrintSupport
)

qt_standard_project_setup()

set(SOURCES
    awards/AwardDefinition.cpp
    awards/AwardDXCC.cpp
    awards/AwardGridsquare.cpp
    awards/AwardIOTA.cpp
    awards/AwardITU.cpp
    awards/AwardJapan.cpp
    awards/AwardNZ.cpp
    awards/AwardRDA.cpp
    awards/AwardSpanishDME.cpp
    awards/AwardUKD.cpp
    awards/AwardUSCounty.cpp
    awards/AwardPOTAActivator.cpp
    awards/AwardPOTAHunter.cpp
    awards/AwardSOTA.cpp
    awards/AwardWAAC.cpp
    awards/AwardWAIP.cpp
    awards/SecondarySubdivisionAward.cpp
    awards/AwardWAC.cpp
    awards/AwardWAS.cpp
    awards/AwardWAZ.cpp
    awards/AwardWPX.cpp
    awards/AwardWWFF.cpp
    awards/BandTableAward.cpp
    core/AlertEvaluator.cpp
    core/AdifRecovery.cpp
    core/AppGuard.cpp
    core/CallbookManager.cpp
    core/CredentialStore.cpp
    core/FileCompressor.cpp
    core/FldigiTCPServer.cpp
    core/LOVDownloader.cpp
    core/LogDatabase.cpp
    core/LogLocale.cpp
    core/LogParam.cpp
    core/MembershipQE.cpp
    core/Migration.cpp
    core/NetworkNotification.cpp
    core/PasswordCipher.cpp
    core/PlatformParameterManager.cpp
    core/PotaQE.cpp
    core/PropConditions.cpp
    core/QSLPrintLabelRenderer.cpp
    core/QSLStorage.cpp
    core/QSOFilterManager.cpp
    core/WsjtxUDPReceiver.cpp
    core/debug.cpp
    core/EmergencyFrequency.cpp
    core/IBPBeacon.cpp
    core/main.cpp
    core/zonedetect.c
    cwkey/CWKeyer.cpp
    cwkey/drivers/CWCatKey.cpp
    cwkey/drivers/CWDaemonKey.cpp
    cwkey/drivers/CWDummyKey.cpp
    cwkey/drivers/CWFldigiKey.cpp
    cwkey/drivers/CWKey.cpp
    cwkey/drivers/CWWinKey.cpp
    data/ActivityProfile.cpp
    data/AntProfile.cpp
    data/BandmapGuide.cpp
    data/BandPlan.cpp
    data/Accents.cpp
    data/CWKeyProfile.cpp
    data/CWShortcutProfile.cpp
    data/Callsign.cpp
    data/Data.cpp
    data/DxServerString.cpp
    data/Gridsquare.cpp
    data/HostsPortString.cpp
    data/MainLayoutProfile.cpp
    data/RigProfile.cpp
    data/RotProfile.cpp
    data/RotUsrButtonsProfile.cpp
    data/SerialPort.cpp
    data/StationProfile.cpp
    data/UpdatableSQLRecord.cpp
    logformat/AdiFormat.cpp
    logformat/AdxFormat.cpp
    logformat/CabrilloFormat.cpp
    logformat/CSVFormat.cpp
    logformat/JsonFormat.cpp
    logformat/LogFormat.cpp
    logformat/PotaAdiFormat.cpp
    models/AlertTableModel.cpp
    models/AwardsTableModel.cpp
    models/DxccTableModel.cpp
    models/LogbookModel.cpp
    models/RigTypeModel.cpp
    models/RotTypeModel.cpp
    models/SearchFilterProxyModel.cpp
    models/ShortcutEditorModel.cpp
    models/SqlListModel.cpp
    models/WsjtxTableModel.cpp
    rig/Rig.cpp
    rig/RigCaps.cpp
    rig/RigctldManager.cpp
    rig/drivers/FlrigRigDrv.cpp
    rig/drivers/GenericRigDrv.cpp
    rig/drivers/HamlibRigDrv.cpp
    rig/drivers/TCIRigDrv.cpp
    rotator/RotCaps.cpp
    rotator/Rotator.cpp
    rotator/drivers/GenericRotDrv.cpp
    rotator/drivers/HamlibRotDrv.cpp
    rotator/drivers/PSTRotDrv.cpp
    service/GenericCallbook.cpp
    service/GenericQSLDownloader.cpp
    service/GenericQSOUploader.cpp
    service/cloudlog/Cloudlog.cpp
    service/clublog/ClubLog.cpp
    service/eqsl/Eqsl.cpp
    service/hamqth/HamQTH.cpp
    service/hrdlog/HRDLog.cpp
    service/kstchat/KSTChat.cpp
    service/lotw/Lotw.cpp
    service/potaapp/PotaApp.cpp
    service/qrzcom/QRZ.cpp
    ui/ActivityEditor.cpp
    ui/AdifRecoveryManager.cpp
    ui/AlertRuleDetail.cpp
    ui/AlertSettingDialog.cpp
    ui/AlertWidget.cpp
    ui/AwardsDialog.cpp
    ui/DXCCSubmissionDialog.cpp
    ui/BandmapWidget.cpp
    ui/BandmapGuideDialog.cpp
    ui/CWConsoleWidget.cpp
    ui/ChatWidget.cpp
    ui/ClockWidget.cpp
    ui/ColumnSettingDialog.cpp
    ui/DevToolsDialog.cpp
    ui/DownloadQSLDialog.cpp
    ui/DxFilterDialog.cpp
    ui/DxWidget.cpp
    ui/DxccTableWidget.cpp
    ui/EditActivitiesDialog.cpp
    ui/CabrilloExportDialog.cpp
    ui/CabrilloTemplateDialog.cpp
    ui/ExportDialog.cpp
    ui/ExportPasswordDialog.cpp
    ui/LoadDatabaseDialog.cpp
    ui/PlatformSettingsDialog.cpp
    ui/QSLGalleryDialog.cpp
    ui/QSLPrintLabelDialog.cpp
    ui/RigctldAdvancedDialog.cpp
    ui/ImportDialog.cpp
    ui/InputPasswordDialog.cpp
    ui/KSTChatWidget.cpp
    ui/KSTHighlightRuleDetail.cpp
    ui/KSTHighlighterSettingDialog.cpp
    ui/LogbookWidget.cpp
    ui/MainWindow.cpp
    ui/MapPageController.cpp
    ui/MapWidget.cpp
    ui/ModeSelectionController.cpp
    ui/NewContactWidget.cpp
    ui/OnlineMapWidget.cpp
    ui/PaperQSLDialog.cpp
    ui/ProfileImageWidget.cpp
    ui/QSLImportStatDialog.cpp
    ui/QSODetailDialog.cpp
    ui/QSOFilterDetail.cpp
    ui/QSOFilterDialog.cpp
    ui/QTableQSOView.cpp
    ui/RigWidget.cpp
    ui/RotatorWidget.cpp
    ui/SettingsDialog.cpp
    ui/ShowUploadDialog.cpp
    ui/StatisticsWidget.cpp
    ui/UploadQSODialog.cpp
    ui/WebEnginePage.cpp
    ui/WsjtxFilterDialog.cpp
    ui/WsjtxWidget.cpp
    ui/component/BaseDoubleSpinBox.cpp
    ui/component/EditLine.cpp
    ui/component/FreqQSpinBox.cpp
    ui/component/ModeSubmodeDelegate.cpp
    ui/component/LogbookFieldComboBox.cpp
    ui/component/MultiselectCompleter.cpp
    ui/component/RepeatButton.cpp
    ui/component/SmartSearchBox.cpp
    ui/component/SqlHighlighter.cpp
    ui/component/SwitchButton.cpp
)

set(HEADERS
    awards/AwardDefinition.h
    awards/AwardDXCC.h
    awards/AwardGridsquare.h
    awards/AwardIOTA.h
    awards/AwardITU.h
    awards/AwardJapan.h
    awards/AwardNZ.h
    awards/AwardRDA.h
    awards/AwardSpanishDME.h
    awards/AwardUKD.h
    awards/AwardUSCounty.h
    awards/AwardPOTAActivator.h
    awards/AwardPOTAHunter.h
    awards/AwardSOTA.h
    awards/AwardWAAC.h
    awards/AwardWAIP.h
    awards/SecondarySubdivisionAward.h
    awards/AwardWAC.h
    awards/AwardWAS.h
    awards/AwardWAZ.h
    awards/AwardWPX.h
    awards/AwardWWFF.h
    awards/BandTableAward.h
    core/AlertEvaluator.h
    core/AdifRecovery.h
    core/AppGuard.h
    core/CallbookManager.h
    core/CredentialStore.h
    core/FileCompressor.h
    core/FldigiTCPServer.h
    core/LOVDownloader.h
    core/LogDatabase.h
    core/LogLocale.h
    core/LogParam.h
    core/MembershipQE.h
    core/Migration.h
    core/NetworkNotification.h
    core/PasswordCipher.h
    core/PlatformParameterManager.h
    core/PotaQE.h
    core/PropConditions.h
    core/QSLPrintLabelRenderer.h
    core/QSLStorage.h
    core/QSOFilterManager.h
    core/QuadKeyCache.h
    core/WsjtxUDPReceiver.h
    core/csv.hpp
    core/debug.h
    core/EmergencyFrequency.h
    core/IBPBeacon.h
    core/zonedetect.h
    cwkey/CWKeyer.h
    cwkey/drivers/CWCatKey.h
    cwkey/drivers/CWDaemonKey.h
    cwkey/drivers/CWDummyKey.h
    cwkey/drivers/CWFldigiKey.h
    cwkey/drivers/CWKey.h
    cwkey/drivers/CWWinKey.h
    data/ActivityProfile.h
    data/AntProfile.h
    data/Band.h
    data/BandmapGuide.h
    data/BandPlan.h
    data/CWKeyProfile.h
    data/CWShortcutProfile.h
    data/Callsign.h
    data/Data.h
    data/DxServerString.h
    data/DxSpot.h
    data/Dxcc.h
    data/Gridsquare.h
    data/HostsPortString.h
    data/MainLayoutProfile.h
    data/POTAEntity.h
    data/POTASpot.h
    data/ProfileManager.h
    data/RigProfile.h
    data/RotProfile.h
    data/RotUsrButtonsProfile.h
    data/SOTAEntity.h
    data/SerialPort.h
    data/SpotAlert.h
    data/StationProfile.h
    data/ToAllSpot.h
    data/UpdatableSQLRecord.h
    data/WCYSpot.h
    data/WWFFEntity.h
    data/WWVSpot.h
    data/WsjtxDecode.h
    data/WsjtxEntry.h
    data/WsjtxLog.h
    data/WsjtxLogADIF.h
    data/WsjtxStatus.h
    logformat/AdiFormat.h
    logformat/AdxFormat.h
    logformat/CabrilloFormat.h
    logformat/CSVFormat.h
    logformat/JsonFormat.h
    logformat/LogFormat.h
    logformat/PotaAdiFormat.h
    models/AlertTableModel.h
    models/AwardsTableModel.h
    models/DxccTableModel.h
    models/LogbookModel.h
    models/RigTypeModel.h
    models/RotTypeModel.h
    models/SearchFilterProxyModel.h
    models/ShortcutEditorModel.h
    models/SqlListModel.h
    models/WsjtxTableModel.h
    rig/Rig.h
    rig/RigCaps.h
    rig/RigctldManager.h
    rig/drivers/FlrigRigDrv.h
    rig/drivers/GenericRigDrv.h
    rig/drivers/HamlibRigDrv.h
    rig/drivers/TCIRigDrv.h
    rig/macros.h
    rotator/RotCaps.h
    rotator/Rotator.h
    rotator/drivers/GenericRotDrv.h
    rotator/drivers/HamlibRotDrv.h
    rotator/drivers/PSTRotDrv.h
    service/GenericCallbook.h
    service/GenericQSLDownloader.h
    service/GenericQSOUploader.h
    service/cloudlog/Cloudlog.h
    service/clublog/ClubLog.h
    service/eqsl/Eqsl.h
    service/hamqth/HamQTH.h
    service/hrdlog/HRDLog.h
    service/kstchat/KSTChat.h
    service/lotw/Lotw.h
    service/potaapp/PotaApp.h
    service/qrzcom/QRZ.h
    ui/ActivityEditor.h
    ui/AdifRecoveryManager.h
    ui/AlertRuleDetail.h
    ui/AlertSettingDialog.h
    ui/AlertWidget.h
    ui/AwardsDialog.h
    ui/DXCCSubmissionDialog.h
    ui/BandmapWidget.h
    ui/BandmapGuideDialog.h
    ui/CWConsoleWidget.h
    ui/ChatWidget.h
    ui/ClockWidget.h
    ui/ColumnSettingDialog.h
    ui/DevToolsDialog.h
    ui/DownloadQSLDialog.h
    ui/DxFilterDialog.h
    ui/DxWidget.h
    ui/DxccTableWidget.h
    ui/EditActivitiesDialog.h
    ui/CabrilloExportDialog.h
    ui/CabrilloTemplateDialog.h
    ui/ExportDialog.h
    ui/ExportPasswordDialog.h
    ui/LoadDatabaseDialog.h
    ui/PlatformSettingsDialog.h
    ui/QSLGalleryDialog.h
    ui/QSLPrintLabelDialog.h
    ui/RigctldAdvancedDialog.h
    ui/ImportDialog.h
    ui/InputPasswordDialog.h
    ui/KSTChatWidget.h
    ui/KSTHighlightRuleDetail.h
    ui/KSTHighlighterSettingDialog.h
    ui/LogbookWidget.h
    ui/MainWindow.h
    ui/MapLayer.h
    ui/MapPageController.h
    ui/MapWidget.h
    ui/ModeSelectionController.h
    ui/NewContactWidget.h
    ui/OnlineMapWidget.h
    ui/PaperQSLDialog.h
    ui/ProfileImageWidget.h
    ui/QSLImportStatDialog.h
    ui/QSODetailDialog.h
    ui/QSOFilterDetail.h
    ui/QSOFilterDialog.h
    ui/QTableQSOView.h
    ui/ShowUploadDialog.h
    ui/SplashScreen.h
    ui/RigWidget.h
    ui/RotatorWidget.h
    ui/SettingsDialog.h
    ui/StatisticsWidget.h
    ui/UploadQSODialog.h
    ui/WebEnginePage.h
    ui/WsjtxFilterDialog.h
    ui/WsjtxWidget.h
    i18n/dbstrings.tri
    i18n/datastrings.tri
    ui/component/BaseDoubleSpinBox.h
    ui/component/ButtonStyle.h
    ui/component/EditLine.h
    ui/component/FreqQSpinBox.h
    ui/component/ModeSubmodeDelegate.h
    ui/component/LogbookFieldComboBox.h
    ui/component/MultiselectCompleter.h
    ui/component/RepeatButton.h
    ui/component/ShutdownAwareWidget.h
    ui/component/SmartSearchBox.h
    ui/component/SqlHighlighter.h
    ui/component/StyleItemDelegate.h
    ui/component/SwitchButton.h
)

set(FORMS
    ui/ActivityEditor.ui
    ui/AlertRuleDetail.ui
    ui/AlertSettingDialog.ui
    ui/AlertWidget.ui
    ui/AwardsDialog.ui
    ui/DXCCSubmissionDialog.ui
    ui/BandmapWidget.ui
    ui/BandmapGuideDialog.ui
    ui/CWConsoleWidget.ui
    ui/ChatWidget.ui
    ui/ClockWidget.ui
    ui/ColumnSettingDialog.ui
    ui/ColumnSettingSimpleDialog.ui
    ui/DevToolsDialog.ui
    ui/DownloadQSLDialog.ui
    ui/DxFilterDialog.ui
    ui/DxWidget.ui
    ui/EditActivitiesDialog.ui
    ui/CabrilloExportDialog.ui
    ui/CabrilloTemplateDialog.ui
    ui/ExportDialog.ui
    ui/ExportPasswordDialog.ui
    ui/LoadDatabaseDialog.ui
    ui/PlatformSettingsDialog.ui
    ui/QSLGalleryDialog.ui
    ui/QSLPrintLabelDialog.ui
    ui/RigctldAdvancedDialog.ui
    ui/ImportDialog.ui
    ui/InputPasswordDialog.ui
    ui/KSTChatWidget.ui
    ui/KSTHighlightRuleDetail.ui
    ui/KSTHighlighterSettingDialog.ui
    ui/LogbookWidget.ui
    ui/MainWindow.ui
    ui/NewContactWidget.ui
    ui/PaperQSLDialog.ui
    ui/ProfileImageWidget.ui
    ui/QSLImportStatDialog.ui
    ui/QSODetailDialog.ui
    ui/QSOFilterDetail.ui
    ui/QSOFilterDialog.ui
    ui/RigWidget.ui
    ui/RotatorWidget.ui
    ui/SettingsDialog.ui
    ui/ShowUploadDialog.ui
    ui/StatisticsWidget.ui
    ui/UploadQSODialog.ui
    ui/WsjtxFilterDialog.ui
    ui/WsjtxWidget.ui
)

set(RESOURCES
    res/flags/flags.qrc
    res/icons/icons.qrc
    res/res.qrc
)

set(OTHER_FILES
    res/stylesheet.css
    res/qlog.rc
    res/qlog.desktop
    res/qlog.1
    res/io.github.foldynl.QLog.metainfo.xml
)

set(TRANSLATIONS
    i18n/qlog_cs.ts
    i18n/qlog_de.ts
    i18n/qlog_es.ts
    i18n/qlog_fr.ts
    i18n/qlog_it.ts
    i18n/qlog_zh_CN.ts
)

qt_add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES}
)

qt_add_translations(${PROJECT_NAME} TS_FILES ${TRANSLATIONS})

find_package(Threads REQUIRED)

if(LINUX)
    find_package(Qt6 REQUIRED COMPONENTS DBus)
    find_package(Qt6Keychain REQUIRED)
    find_package(OpenSSL REQUIRED)

    find_package(PkgConfig REQUIRED)

    pkg_check_modules(HAMLIB REQUIRED IMPORTED_TARGET hamlib)
    pkg_check_modules(SQLITE3 REQUIRED IMPORTED_TARGET sqlite3)
    pkg_check_modules(ZLIB REQUIRED IMPORTED_TARGET zlib)
elseif(WIN32)
    target_sources(${PROJECT_NAME} SOURCES
        rig/drivers/OmnirigRigDrv.cpp
        rig/drivers/Omnirigv2RigDrv.cpp
    )

    target_sources(${PROJECT_NAME} HEADERS
        rig/drivers/OmnirigRigDrv.h
        rig/drivers/Omnirigv2RigDrv.h
        rig/drivers/OmniRigEventSink.h
    )

    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIN32_LEAN_AND_MEAN
        ZD_EXPORT=
    )

    target_include_directories(${PROJECT_NAME} PRIVATE
        /usr/local/include
    )

    # Add sqlite3

    find_library(WS2_32 ws2_32 REQUIRED)
    find_library(HAMLIB libhamlib-4 REQUIRED)
    find_library(ZLIB zlib REQUIRED)
    find_library(CRYPTO libcrypto REQUIRED)
    find_library(SSL ssl REQUIRED)
    find_library(QT6KEYCHAIN qt6keychain REQUIRED)
elseif(APPLE)
    target_include_directories(${PROJECT_NAME} PRIVATE
        /usr/local/include
        /opt/homebrew/include
        /opt/local/include
    )

    set(LIB_SEARCH_PATHS
        /usr/local/lib
        /opt/homebrew/lib
        /opt/local/lib
    )

    target_link_directories(${PROJECT_NAME} PRIVATE ${LIB_SEARCH_PATHS})

    find_library(SQLITE3 sqlite3 REQUIRED PATHS ${LIB_SEARCH_PATHS})
    find_library(HAMLIB hamlib REQUIRED PATHS ${LIB_SEARCH_PATHS})
    find_library(ZLIB z REQUIRED PATHS ${LIB_SEARCH_PATHS})
    find_library(CRYPTO crypto REQUIRED PATHS ${LIB_SEARCH_PATHS})
    find_library(SSL ssl REQUIRED  PATHS ${LIB_SEARCH_PATHS})
    find_library(QT6KEYCHAIN qt6keychain REQUIRED  PATHS ${LIB_SEARCH_PATHS})
endif()

if(LINUX)
    string(REPLACE "." ";" HAMLIB_VERSION_LIST ${HAMLIB_VERSION})

    list(GET ${HAMLIB_VERSION_LIST} 0 HAMLIBVERSION_MAJOR)
    list(GET ${HAMLIB_VERSION_LIST} 1 HAMLIBVERSION_MINOR)
    list(GET ${HAMLIB_VERSION_LIST} 2 HAMLIBVERSION_PATCH)

    add_compile_definitions(
        HAMLIBVERSION_MAJOR=${HAMLIBVERSION_MAJOR}
        HAMLIBVERSION_MINOR=${HAMLIBVERSION_MINOR}
        HAMLIBVERSION_PATCH=${HAMLIBVERSION_PATCH}
    )
elseif(APPLE OR WIN32)
    string(REPLACE "." ";" HAMLIB_VERSION_LIST ${HAMLIB})
    add_compile_definitions()
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}
)

# Linking

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Xml
    Qt6::Charts
    Qt6::WebEngineWidgets
    Qt6::SerialPort
    Qt6::QuickWidgets
    Qt6::WebChannel
    Qt6::WebSockets
    Qt6::PrintSupport
)

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

if(LINUX)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::DBus
        Qt6Keychain::Qt6Keychain
        OpenSSL
        PkgConfig::HAMLIB
        PkgConfig::SQLITE3
        PkgConfig::ZLIB
    )
elseif(WIN32 OR APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        HAMLIB
        ZLIB
        SSL
        CRYPTO
        QT6KEYCHAIN
    )

    if(WIN32)
        target_link_libraries(${PROJECT_NAME} PRIVATE WS2_32)
    elseif(APPLE)
        target_link_libraries(${PROJECT_NAME} PRIVATE SQLITE3)
    endif()
endif()

# Installation rules

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE OFF
)

if(LINUX)
    include(GNUInstallDirs)

    install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(FILES ${CMAKE_SOURCE_DIR}/res/${PROJECT_NAME}.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )

    install(FILES ${CMAKE_SOURCE_DIR}/res/${PROJECT_NAME}.1
        DESTINATION ${CMAKE_INSTALL_DATADIR}/man/man1
    )

    install(FILES ${CMAKE_SOURCE_DIR}/res/${PROJECT_NAME}.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
    )

    install(FILES ${CMAKE_SOURCE_DIR}/res/io.github.foldynl.QLog.metainfo.xml
        DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo
    )
endif()