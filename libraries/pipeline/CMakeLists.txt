add_library(pipeline
    inc/PipelineStage.h
    inc/ConsumerStage.h
    inc/SPMCStageConnection.h
    inc/InStageConnection.h
    inc/OutStageConnection.h
    inc/Pipeline.h
    inc/IPipelineStage.h
    inc/ProducerAndConsumerStage.h
    inc/ProducerStage.h
    inc/StageConnection.h
    inc/StageTask.h
    inc/ConsumptionStrategy.h
    inc/PipelineException.h
    inc/PipelineStageType.h
    inc/StageTaskState.h
    inc/PipelineHelpers.h
    
    src/Pipeline.cpp
    src/IPipelineStage.cpp
    src/PipelineHelpers.cpp)

target_link_libraries(pipeline PUBLIC common)
target_include_directories(pipeline PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
