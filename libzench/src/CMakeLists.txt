# top-level namespace source files
target_sources(
    libzench
        PRIVATE
            Instruction.cpp
            VariableProxy.cpp
            zench.cpp
            ZMachine.cpp
            ZMachineImpl.cpp
            ZStringDecoder.cpp
)
# sub-namespace source directories
# NOTE: none yet!
