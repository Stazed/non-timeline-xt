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

if(BuildFLTK)
    set(FLTK_PanZoomer  ../FL/Fl_Panzoomer.C)
endif(BuildFLTK)

add_executable (non-timeline-xt ${ProgSources} ${FLTK_PanZoomer} src/main.C)

if(BuildFLTK)
    set(ExternLibraries
        ${FLTK_LIBRARIES}
        ${FLTK_IMAGES_LIBRARY}
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
        ${JACK_INCLUDE_DIRS}
        ${SNDFILE_INCLUDE_DIRS}
        ${LIBLO_INCLUDE_DIRS}
    )
#NTK
else(BuildFLTK)
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
endif(BuildFLTK)

target_link_libraries (non-timeline-xt ${ExternLibraries})
install (TARGETS non-timeline-xt RUNTIME DESTINATION bin)


install (FILES non-timeline.desktop.in
    DESTINATION share/applications RENAME non-timeline-xt.desktop)

install (PROGRAMS bin/import-external-sources.sh DESTINATION bin RENAME import-external-sources)
install (PROGRAMS bin/remove-unused-sources.sh DESTINATION bin RENAME remove-unused-sources)
