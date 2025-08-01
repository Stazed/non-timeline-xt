#CMake file for non-timeline-xt

project (non-timeline-xt)

set (ProgSources
    ../nonlib/JACK/Client.C
    ../nonlib/JACK/Port.C
    ../nonlib/Log_Entry.C
    ../nonlib/Loggable.C
    ../nonlib/NSM/Client.C
    ../nonlib/OSC/Endpoint.C
    ../nonlib/Thread.C
    ../nonlib/debug.C
    ../nonlib/dsp.C
    ../nonlib/file.C
    ../nonlib/string_util.C

    ../FL/About_Dialog.C
    ../FL/Fl_Menu_Settings.C
    ../FL/Fl_Scalepack.C
    ../FL/Fl_Text_Edit_Window.C
    ../FL/New_Project_Dialog.C
    ../FL/menu_popup.C
    ../FL/test_press.C
    ../FL/focus_frame.C

    src/Annotation_Region.C
    src/Audio_Region.C
    src/Audio_Sequence.C
    src/Control_Point.C
    src/Control_Sequence.C
    src/Cursor_Point.C
    src/Cursor_Region.C
    src/Cursor_Sequence.C
    src/Engine/Audio_File.C
    src/Engine/Audio_File_Dummy.C
    src/Engine/Audio_File_SF.C
    src/Engine/Audio_Region.C
    src/Engine/Audio_Sequence.C
    src/Engine/Control_Sequence.C
    src/Engine/Disk_Stream.C
    src/Engine/Engine.C
    src/Engine/Peaks.C
    src/Engine/Playback_DS.C
    src/Engine/Record_DS.C
    src/Engine/Timeline.C
    src/Engine/Track.C
    src/NSM.C
    src/OSC_Transmit_Thread.C
    src/OSC_Receive_Thread.C
    src/Project.C
    src/Region_Volume_Editor.C
    src/Sequence.C
    src/Sequence_Point.C
    src/Sequence_Region.C
    src/Sequence_Widget.C
    src/TLE.C
    src/Track_Header.C
    src/Tempo_Point.C
    src/Tempo_Sequence.C
    src/Time_Point.C
    src/Time_Sequence.C
    src/Timeline.C
    src/Track.C
    src/Transport.C
    src/Waveform.C
    src/main.C
)

if(EnableNTKStatic)
    message (STATUS "USING NTK STATIC BUILD")
else(EnableNTKStatic)
    if(EnableNTK)
        message (STATUS "USING NTK DYNAMIC BUILD")
    else(EnableNTK) #FLTK
        message (STATUS "USING FLTK BUILD")
        set(FLTK_specific
            ../FL/Fl_Panzoomer.C
            ../FL/Fl_Theme_Chooser.cxx
            ../FL/Fl_Theme.cxx
            ../FL/GTK_Theme.cxx
            ../FL/Clean_Theme.cxx
            ../FL/Crystal_Theme.cxx
            ../FL/Gleam_Theme.cxx
            ../FL/Vector_Theme.cxx
            ../FL/themes.cxx
        )
    endif(EnableNTK)
endif(EnableNTKStatic)

add_executable (non-timeline-xt ${ProgSources} ${FLTK_specific} src/main.C)

if(EnableNTKStatic)
    set(ExternLibraries
        ${FONTCONFIG_LIBRARIES}
        ${JACK_LINK_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${SNDFILE_LIBRARIES}
        ${X11_LIBRARIES}
        ${LIBLO_LIBRARIES}
        ${XFT_LIBRARIES}
        ${XRENDER_LIBRARIES}
        ${XINERAMA_LIBRARIES}
        cairo
        Xcursor
        Xfixes
        jpeg
        png
        dl
        m
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

else(EnableNTKStatic)
    if(EnableNTK)   #NTK dynamic
        set(ExternLibraries
            ${NTK_IMAGES_LIBRARIES}
            ${NTK_LIBRARIES}
            ${JACK_LINK_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT}
            ${SNDFILE_LIBRARIES}
            ${LIBLO_LIBRARIES}
            dl
            m
        )

        target_include_directories (
            non-timeline-xt PRIVATE
            ${NTK_INCLUDE_DIRS}
            ${JACK_INCLUDE_DIRS}
            ${SNDFILE_INCLUDE_DIRS}
            ${LIBLO_INCLUDE_DIRS}
        )

    else(EnableNTK) #FLTK
        if(EnableFLTKStatic)
            set(ExternLibraries
                ${LIBCAIRO_LIBRARIES}
                ${JACK_LINK_LIBRARIES}
                ${CMAKE_THREAD_LIBS_INIT}
                ${SNDFILE_LIBRARIES}
                ${X11_LIBRARIES}
                ${LIBLO_LIBRARIES}
                ${FONTCONFIG_LIBRARIES}
                ${XFT_LIBRARIES}
                ${XRENDER_LIBRARIES}
                ${XINERAMA_LIBRARIES}
                fontconfig
                Xcursor
                Xfixes
                jpeg
                png
                dl
                m
                z   # needed for fltk-1.4 static build
            )

        target_include_directories (
            non-timeline-xt PRIVATE
            ${FLTK_INCLUDE_DIR}
            ${LIBCAIRO_INCLUDE_DIRS}
            ${JACK_INCLUDE_DIRS}
            ${SNDFILE_INCLUDE_DIRS}
            ${LIBLO_INCLUDE_DIRS}
            ${FONTCONFIG_INCLUDE_DIRS}
            ${XFT_INCLUDE_DIRS}
            ${XRENDER_INCLUDE_DIRS}
            ${XINERAMA_INCLUDE_DIRS}
        )
        else(EnableFLTKStatic)  # Dynamic
            set(ExternLibraries
                ${FLTK_LIBRARIES}
                ${FLTK_IMAGES_LIBRARY}
                ${LIBCAIRO_LIBRARIES}
                ${JACK_LINK_LIBRARIES}
                ${CMAKE_THREAD_LIBS_INIT}
                ${SNDFILE_LIBRARIES}
                ${LIBLO_LIBRARIES}
                dl
                m
            )

            target_include_directories (
                non-timeline-xt PRIVATE
                ${FLTK_INCLUDE_DIR}
                ${LIBCAIRO_INCLUDE_DIRS}
                ${JACK_INCLUDE_DIRS}
                ${SNDFILE_INCLUDE_DIRS}
                ${LIBLO_INCLUDE_DIRS}
            )
        endif(EnableFLTKStatic)
    endif(EnableNTK)
endif(EnableNTKStatic)

if(EnableFLTKStatic)
    target_link_libraries (non-timeline-xt ${FLTK_STATIC} ${FLTK_STATIC_IMAGES} ${ExternLibraries})
else(EnableFLTKStatic)  # NTK
    if(EnableNTKStatic)
        target_link_libraries (non-timeline-xt ${NTK_STATIC} ${NTK_STATIC_IMAGES} ${ExternLibraries})
    else(EnableNTKStatic)   # NTK dynamic
        target_link_libraries (non-timeline-xt ${ExternLibraries})
    endif(EnableNTKStatic)
endif(EnableFLTKStatic)

install (TARGETS non-timeline-xt RUNTIME DESTINATION bin)


install (FILES non-timeline.desktop.in
    DESTINATION share/applications RENAME non-timeline-xt.desktop)

install (PROGRAMS bin/import-external-sources.sh DESTINATION bin RENAME import-external-sources)
install (PROGRAMS bin/remove-unused-sources.sh DESTINATION bin RENAME remove-unused-sources)
