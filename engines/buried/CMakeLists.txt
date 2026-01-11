add_library(buried STATIC)

target_sources(
  buried
  PRIVATE agent_evaluation.cpp
          avi_frames.cpp
          biochip_right.cpp
          biochip_view.cpp
          buried.cpp
          complete.cpp
          console.cpp
          credits.cpp
          death.cpp
          frame_window.cpp
          gameui.cpp
          graphics.cpp
          inventory_info.cpp
          inventory_window.cpp
          livetext.cpp
          main_menu.cpp
          metaengine.cpp
          navarrow.cpp
          overview.cpp
          saveload.cpp
          scene_view.cpp
          sound.cpp
          title_sequence.cpp
          video_window.cpp
          window.cpp
          demo/demo_menu.cpp
          demo/features.cpp
          demo/movie_scene.cpp
          environ/agent3_lair.cpp
          environ/ai_lab.cpp
          environ/alien.cpp
          environ/castle.cpp
          environ/da_vinci.cpp
          environ/future_apartment.cpp
          environ/mayan.cpp
          environ/scene_base.cpp
          environ/scene_common.cpp
          environ/scene_factory.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          agent_evaluation.h
          avi_frames.h
          biochip_right.h
          biochip_view.h
          buried.h
          complete.h
          console.h
          credits.h
          death.h
          frame_window.h
          gameui.h
          graphics.h
          inventory_info.h
          inventory_window.h
          livetext.h
          main_menu.h
          navarrow.h
          overview.h
          scene_view.h
          sound.h
          title_sequence.h
          video_window.h
          window.h
          demo/demo_menu.h
          demo/features.h
          demo/movie_scene.h
          environ/scene_base.h
          environ/scene_common.h
)

add_library(buried_detection)

target_sources(buried_detection PRIVATE detection.cpp)
