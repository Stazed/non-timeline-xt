#CMake file for non-timeline-xt

project (non-timeline-xt)

set(NONLIB_SOURCES
    ${CMAKE_SOURCE_DIR}/nonlib/JACK/Client.C
    ${CMAKE_SOURCE_DIR}/nonlib/JACK/Port.C
    ${CMAKE_SOURCE_DIR}/nonlib/Log_Entry.C
    ${CMAKE_SOURCE_DIR}/nonlib/Loggable.C
    ${CMAKE_SOURCE_DIR}/nonlib/NSM/Client.C
    ${CMAKE_SOURCE_DIR}/nonlib/OSC/Endpoint.C
    ${CMAKE_SOURCE_DIR}/nonlib/Thread.C
    ${CMAKE_SOURCE_DIR}/nonlib/debug.C
    ${CMAKE_SOURCE_DIR}/nonlib/dsp.C
    ${CMAKE_SOURCE_DIR}/nonlib/file.C
    ${CMAKE_SOURCE_DIR}/nonlib/string_util.C
)

set(FL_SOURCES
    ${CMAKE_SOURCE_DIR}/FL/About_Dialog.C
    ${CMAKE_SOURCE_DIR}/FL/Fl_Menu_Settings.C
    ${CMAKE_SOURCE_DIR}/FL/Fl_Scalepack.C
    ${CMAKE_SOURCE_DIR}/FL/Fl_Text_Edit_Window.C
    ${CMAKE_SOURCE_DIR}/FL/New_Project_Dialog.C
    ${CMAKE_SOURCE_DIR}/FL/menu_popup.C
    ${CMAKE_SOURCE_DIR}/FL/test_press.C
    ${CMAKE_SOURCE_DIR}/FL/focus_frame.C
)

set(CORE_SOURCES
    ${CMAKE_SOURCE_DIR}/timeline/src/Annotation_Region.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Audio_Region.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Audio_Sequence.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Control_Point.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Control_Sequence.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Cursor_Point.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Cursor_Region.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Cursor_Sequence.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Engine/Audio_File.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Engine/Audio_File_Dummy.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Engine/Audio_File_SF.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Engine/Audio_Region.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Engine/Audio_Sequence.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Engine/Control_Sequence.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Engine/Disk_Stream.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Engine/Engine.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Engine/Peaks.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Engine/Playback_DS.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Engine/Record_DS.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Engine/Timeline.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Engine/Track.C
    ${CMAKE_SOURCE_DIR}/timeline/src/NSM.C
    ${CMAKE_SOURCE_DIR}/timeline/src/OSC_Transmit_Thread.C
    ${CMAKE_SOURCE_DIR}/timeline/src/OSC_Receive_Thread.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Project.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Region_Volume_Editor.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Sequence.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Sequence_Point.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Sequence_Region.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Sequence_Widget.C
    ${CMAKE_SOURCE_DIR}/timeline/src/TLE.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Track_Header.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Tempo_Point.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Tempo_Sequence.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Time_Point.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Time_Sequence.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Timeline.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Track.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Transport.C
    ${CMAKE_SOURCE_DIR}/timeline/src/Waveform.C
    ${CMAKE_SOURCE_DIR}/timeline/src/main.C
)

set(ProgSources
    ${NONLIB_SOURCES}
    ${FL_SOURCES}
    ${CORE_SOURCES}
)

if(EnableFLTK OR EnableFLTKStatic)
    message (STATUS "USING FLTK BUILD")
    set(FLTK_specific
        ${CMAKE_SOURCE_DIR}/FL/Fl_Panzoomer.C
        ${CMAKE_SOURCE_DIR}/FL/Fl_Theme_Chooser.cxx
        ${CMAKE_SOURCE_DIR}/FL/Fl_Theme.cxx
        ${CMAKE_SOURCE_DIR}/FL/GTK_Theme.cxx
        ${CMAKE_SOURCE_DIR}/FL/Clean_Theme.cxx
        ${CMAKE_SOURCE_DIR}/FL/Crystal_Theme.cxx
        ${CMAKE_SOURCE_DIR}/FL/Gleam_Theme.cxx
        ${CMAKE_SOURCE_DIR}/FL/Vector_Theme.cxx
        ${CMAKE_SOURCE_DIR}/FL/themes.cxx
    )
endif()

set(NTK_STATIC_FLTK_LEGACY_LIBS
    ${FONTCONFIG_LIBRARIES}
    ${JACK_LINK_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${SNDFILE_LIBRARIES}
    ${X11_LIBRARIES}
    ${LIBLO_LIBRARIES}
    ${XFT_LIBRARIES}
    ${XRENDER_LIBRARIES}
    ${XINERAMA_LIBRARIES}
    ${CAIRO_LIBRARIES}
    ${XCURSOR_LIBRARIES}
    ${XFIXES_LIBRARIES}
    ${JPEG_LIBRARY}
    ${LIBPNG_LIBRARIES}
    ${DL_LIBRARY}
    ${M_LIBRARY}
)

add_executable (non-timeline-xt ${ProgSources} ${FLTK_specific} ${CMAKE_SOURCE_DIR}/timeline/src/main.C)

# FLTK shared build - default
if(EnableFLTK)
    if(TARGET fltk::fltk-shared)    # Modern cmake
        set(ExternLibraries
            ${CAIRO_LIBRARIES}
            ${JACK_LINK_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT}
            ${SNDFILE_LIBRARIES}
            ${LIBLO_LIBRARIES}
            ${DL_LIBRARY}
            ${M_LIBRARY}
        )

        target_include_directories (
            non-timeline-xt PRIVATE
            ${CAIRO_INCLUDE_DIRS}
            ${JACK_INCLUDE_DIRS}
            ${SNDFILE_INCLUDE_DIRS}
            ${LIBLO_INCLUDE_DIRS}
        )
    else()                          # Legacy cmake
        set(ExternLibraries
            ${FLTK_LIBRARIES}
            ${FLTK_IMAGES_LIBRARY}
            ${CAIRO_LIBRARIES}
            ${JACK_LINK_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT}
            ${SNDFILE_LIBRARIES}
            ${LIBLO_LIBRARIES}
            ${DL_LIBRARY}
            ${M_LIBRARY}
        )

        target_include_directories (
            non-timeline-xt PRIVATE
            ${FLTK_INCLUDE_DIR}
            ${CAIRO_INCLUDE_DIRS}
            ${JACK_INCLUDE_DIRS}
            ${SNDFILE_INCLUDE_DIRS}
            ${LIBLO_INCLUDE_DIRS}
        )
    endif()
endif(EnableFLTK)

if(EnableFLTKStatic)
    if(TARGET fltk::fltk)   # Modern cmake
        set(ExternLibraries
            ${CAIRO_LIBRARIES}
            ${JACK_LINK_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT}
            ${SNDFILE_LIBRARIES}
            ${LIBLO_LIBRARIES}
            ${DL_LIBRARY}
            ${M_LIBRARY}
        )
        target_include_directories (
            non-timeline-xt PRIVATE
            ${CAIRO_INCLUDE_DIRS}
            ${JACK_INCLUDE_DIRS}
            ${SNDFILE_INCLUDE_DIRS}
            ${LIBLO_INCLUDE_DIRS}
        )
    else()                  # Legacy cmake
        set(ExternLibraries
            ${NTK_STATIC_FLTK_LEGACY_LIBS}
        )

        target_include_directories (
            non-timeline-xt PRIVATE
            ${FLTK_INCLUDE_DIR}
            ${CAIRO_INCLUDE_DIRS}
            ${JACK_INCLUDE_DIRS}
            ${SNDFILE_INCLUDE_DIRS}
            ${LIBLO_INCLUDE_DIRS}
            ${FONTCONFIG_INCLUDE_DIRS}
            ${XFT_INCLUDE_DIRS}
            ${XRENDER_INCLUDE_DIRS}
            ${XINERAMA_INCLUDE_DIRS}
        )
    endif()
endif(EnableFLTKStatic)

# NTK shared
if(EnableNTK)
    set(ExternLibraries
        ${NTK_IMAGES_LIBRARIES}
        ${NTK_LIBRARIES}
        ${JACK_LINK_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${SNDFILE_LIBRARIES}
        ${LIBLO_LIBRARIES}
        ${DL_LIBRARY}
        ${M_LIBRARY}
    )

    target_include_directories (
        non-timeline-xt PRIVATE
        ${NTK_INCLUDE_DIRS}
        ${JACK_INCLUDE_DIRS}
        ${SNDFILE_INCLUDE_DIRS}
        ${LIBLO_INCLUDE_DIRS}
    )
endif(EnableNTK)

if(EnableNTKStatic)
    set(ExternLibraries
        ${NTK_STATIC_FLTK_LEGACY_LIBS}
    )

    target_include_directories (
        non-timeline-xt PRIVATE
        ${NTK_INCLUDE_DIRS}
        ${FONTCONFIG_INCLUDE_DIRS}
        ${JACK_INCLUDE_DIRS}
        ${SNDFILE_INCLUDE_DIRS}
        ${LIBLO_INCLUDE_DIRS}
        ${XFT_INCLUDE_DIRS}
        ${XRENDER_INCLUDE_DIRS}
        ${XINERAMA_INCLUDE_DIRS}
    )
endif(EnableNTKStatic)

if(EnableFLTK)
    if(TARGET fltk::fltk-shared)    # Modern cmake
        target_link_libraries (non-timeline-xt PRIVATE fltk::fltk-shared fltk::images ${ExternLibraries})
    else()
        target_link_libraries (non-timeline-xt PRIVATE ${ExternLibraries})
    endif()
endif()

if(EnableNTK)
    target_link_libraries (non-timeline-xt PRIVATE ${ExternLibraries})
endif()

if(EnableFLTKStatic)
    if(TARGET fltk::fltk)   # Modern cmake
        target_link_libraries (non-timeline-xt PRIVATE fltk::fltk fltk::images ${ExternLibraries})
    else()                  # Legacy
        target_link_libraries (non-timeline-xt PRIVATE ${FLTK_STATIC} ${FLTK_STATIC_IMAGES} ${ExternLibraries})
    endif()
endif()

if(EnableNTKStatic)
    target_link_libraries (non-timeline-xt PRIVATE ${NTK_STATIC} ${NTK_STATIC_IMAGES} ${ExternLibraries})
endif()


install (TARGETS non-timeline-xt RUNTIME DESTINATION bin)


install (FILES non-timeline.desktop.in
    DESTINATION share/applications RENAME non-timeline-xt.desktop)

install (PROGRAMS bin/import-external-sources.sh DESTINATION bin RENAME import-external-sources)
install (PROGRAMS bin/remove-unused-sources.sh DESTINATION bin RENAME remove-unused-sources)
