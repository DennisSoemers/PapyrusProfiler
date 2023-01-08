Scriptname PapyrusProfilerQuestScript extends nl_cmd_module  

Event OnInit()
	If (!nl_cmd_globalinfo.IsInstalled())
		Debug.Trace("NL_CMD is not installed!")
		return
	EndIf

	RegisterCommand("StartPapyrusProfiling", "OnStartProfiling", desc="Starts the Papyrus Profiler. When no configuration is specified, it will keep running until the StopPapyrusProfiling command is executed.")
	RegisterCommand("StartPapyrusProfilingConfig", "OnStartProfilingConfig", "string", desc="Starts the Papyrus Profiler with the given configuration. Depending on the configuration used, it may or may not terminate automatically at some point.")
	RegisterCommand("StopPapyrusProfiling", "OnStopProfiling", desc="Stops the Papyrus Profiler.")
EndEvent

Event OnStartProfiling()
	OnStartProfilingConfig("")
EndEvent

Event OnStartProfilingConfig(string config)
	; TODO
EndEvent

Event OnStopProfiling()
	; TODO
EndEvent