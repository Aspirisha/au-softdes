package au.sdshell.driver.command;

import au.sdshell.common.Environment;
import au.sdshell.common.FileResolver;
import javafx.util.Pair;

import java.io.IOException;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * Created by andy on 9/16/16.
 */
public class ToolCommand {
    private String commandName;
    final List<String> argumentsDescription; // each entry here is space-free string with substituted environment variables
    boolean isPiped;

    public enum ToolCommandStatus {
        TOOL_NOT_FOUND,
        EXECUTION_FAILURE,
        SUCCESS
    }

    public ToolCommand(String name, List<String> args, boolean isPiped) {
        commandName = name;
        argumentsDescription = args;
        this.isPiped = isPiped;
    }

    public String getCommandName() {
        return commandName;
    }

    public ToolCommandStatus run() {
        Pair<ProcessBuilder, ToolCommandStatus> pbAndStatus = prepareCommand();
        if (pbAndStatus.getValue() != ToolCommandStatus.SUCCESS) {
            return pbAndStatus.getValue();
        }

        ProcessBuilder pb = pbAndStatus.getKey();
        pb.inheritIO();
        Process p;
        try {
            p = pb.start();
        } catch (IOException e) {
            return ToolCommandStatus.EXECUTION_FAILURE;
        }

        while (p.isAlive()) {
            try {
                p.waitFor();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        return ToolCommandStatus.SUCCESS;
    }

    public Pair<Process, ToolCommandStatus> prepareCommandForPiping() {
        Pair<ProcessBuilder, ToolCommandStatus> pbAndStatus = prepareCommand();

        if (pbAndStatus.getValue() != ToolCommandStatus.SUCCESS) {
            return new Pair<>(null, pbAndStatus.getValue());
        }

        try {
            return new Pair<>(pbAndStatus.getKey().start(), ToolCommandStatus.SUCCESS);
        } catch (IOException e) {
            return new Pair<>(null, ToolCommandStatus.EXECUTION_FAILURE);
        }
    }

    public Pair<ProcessBuilder, ToolCommandStatus> prepareCommand() {
        commandName = Environment.substituteVariables(commandName);
        Path tool = FileResolver.findFile(commandName);

        if (tool == null) {
            return new Pair<>(null, ToolCommandStatus.TOOL_NOT_FOUND);
        }

        List<String> commandList = new LinkedList<>(Arrays.asList("java", "-jar", tool.toString()));
        commandList.addAll(argumentsDescription
                .stream()
                .map(Environment::substituteVariables)
                .collect(Collectors.toList()));

        String[] commandArray = new String[commandList.size()];
        ProcessBuilder pb = new ProcessBuilder(commandList.toArray(commandArray));

        Map<String, String> env = pb.environment();
        env.clear();
        env.putAll(Environment.getInstance().getEnvironmentMap());

        return new Pair<>(pb, ToolCommandStatus.SUCCESS);
    }
}
