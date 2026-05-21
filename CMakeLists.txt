cmake_minimum_required(VERSION 3.16)

project(auraTest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Svg Network Sql WebSockets)

# Добавляем все исходные файлы (новая структура)
set(PROJECT_SOURCES
    src/main.cpp
    src/ui/mainwindow.cpp
    src/ui/titlebar.cpp
    src/ui/leftnavigation.cpp
    src/ui/middlepanel.cpp
    src/ui/rightpanel.cpp
    src/ui/dialogs/settingsdialog.cpp
    src/ui/dialogs/supportdialog.cpp
    src/ui/widgets/togglebutton.cpp
    src/ui/widgets/FlowLayout.cpp
    src/ui/widgets/FlowLayout.h
    src/core/utils/stylehelper.cpp
    src/network/NetworkManager.cpp
    src/network/NetworkManager.h
    roundbuttons.h
    src/ui/dialogs/logindialog.cpp
    src/ui/dialogs/logindialog.h
    src/ui/dialogs/overlaydialog.cpp
    src/ui/dialogs/overlaydialog.h
    src/ui/panels/taskPanel/TasksPanel.cpp
    src/ui/panels/taskPanel/TasksPanel.h
    src/ui/panels/taskPanel/TaskDialog.h
    src/ui/panels/taskPanel/TaskDialog.cpp
    src/ui/panels/taskPanel/TaskItemWidget.h
    src/ui/panels/taskPanel/Task.h
    src/ui/panels/taskPanel/TaskItemWidget.cpp
    src/ui/panels/taskPanel/TaskStorage.h
    src/ui/panels/taskPanel/TaskStorage.cpp
    src/ui/panels/dealPanel/Deal.h
    src/ui/panels/dealPanel/DealDialog.cpp
    src/ui/panels/dealPanel/DealDialog.h
    src/ui/panels/dealPanel/DealsPanel.cpp
    src/ui/panels/dealPanel/DealsPanel.h
    src/ui/Navigation.h
    src/ui/dialogs/ClientDetailDialog.h
    src/ui/dialogs/ClientDetailDialog.cpp
    src/ui/panels/documentsPanel/DocumentsPanel.cpp
    src/ui/panels/documentsPanel/DocumentsPanel.h
    src/ui/dialogs/UploadDocumentDialog.cpp
    src/ui/dialogs/UploadDocumentDialog.h
    src/ui/dialogs/CompanyDetailDialog.cpp
    src/ui/dialogs/CompanyDetailDialog.h
    src/ui/dialogs/CompanyDialog.cpp
    src/ui/dialogs/CompanyDialog.h
    src/ui/panels/companiesPanel/CompaniesPanel.cpp
    src/ui/panels/companiesPanel/CompaniesPanel.h
    src/ui/panels/SidePanel.cpp
    src/ui/panels/SidePanel.h
    src/ui/panels/employeesPanel/EmployeesPanel.cpp
    src/ui/panels/employeesPanel/EmployeesPanel.h
    src/ui/dialogs/ProfileDialog.cpp
    src/ui/dialogs/ProfileDialog.h
    src/ui/dialogs/EditProfileDialog.h
    src/ui/dialogs/EditProfileDialog.cpp
    src/ui/PhotoViewer.cpp
    src/ui/PhotoViewer.h
    src/ui/panels/taskPanel/EditTaskDialog.cpp
    src/ui/panels/taskPanel/EditTaskDialog.h
    src/ui/panels/calendarPanel/CalendarPanel.cpp
    src/ui/panels/calendarPanel/CalendarPanel.h
    src/ui/panels/calendarPanel/EventDialog.h
    src/ui/panels/calendarPanel/EventDialog.cpp
    src/ui/panels/calendarPanel/EventDetailDialog.cpp
    src/ui/panels/calendarPanel/EventDetailDialog.h
    src/ui/dialogs/UserProfileDialog.cpp
    src/ui/dialogs/UserProfileDialog.h
    src/ui/dialogs/newticketdialog.h
    src/ui/dialogs/newticketdialog.cpp
)

# Подключаем ресурсы
set(PROJECT_RESOURCES
    resources.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(auraTest
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${PROJECT_RESOURCES}
    )
else()
    if(ANDROID)
        add_library(auraTest SHARED
            ${PROJECT_SOURCES}
            ${PROJECT_RESOURCES}
        )
    else()
        add_executable(auraTest
            ${PROJECT_SOURCES}
            ${PROJECT_RESOURCES}
        )
    endif()
endif()

target_link_libraries(auraTest PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::Core Qt6::Svg Qt6::Network Qt6::Widgets Qt6::Sql Qt6::WebSockets)

# Добавляем пути для include
target_include_directories(auraTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/dialogs
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/utils
)

if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.auraTest)
endif()
set_target_properties(auraTest PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS auraTest
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(auraTest)
endif()

if(WIN32)
    target_compile_definitions(auraTest PRIVATE Q_OS_WIN)
endif()

if(WIN32)
    target_link_libraries(auraTest PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::Core Qt6::Svg dwmapi)
else()
    target_link_libraries(auraTest PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::Core Qt6::Svg)
endif()
