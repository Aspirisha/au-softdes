package au.sdshell.driver.command;

import au.sdshell.common.Environment;
import au.sdshell.common.FileResolver;
import au.sdshell.driver.Driver;
import javafx.util.Pair;

import java.io.*;
import java.nio.file.Path;
import java.util.*;
import java.util.stream.Collectors;

/**
 * Created by andy on 9/16/16.
 */
public class ToolCommand {
    private static final String toolsRegisterFile = Arrays.asList(Driver.rootDirectory, ".tools")
            .stream().collect(Collectors.joining(File.separator));
    private static Set<String> registeredCommands;

    static {
        registeredCommands = new HashSet<>();

        try (BufferedReader br = new BufferedReader(new FileReader(toolsRegisterFile))) {
            String sCurrentLine;

            while ((sCurrentLine = br.readLine()) != null) {
                registeredCommands.add(sCurrentLine);
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private String commandName;
    final List<String> argumentsDescription;


    public enum ToolCommandStatus {
        TOOL_NOT_FOUND,
        EXECUTION_FAILURE,
        SUCCESS
    }

    /**
     * @param name command name
     *             the path to the command is resolved via {@link FileResolver}
     * @param args command arguments
     *             substitution of environment variables is done vi {@link Environment}
     */
    public ToolCommand(String name, List<String> args) {
        commandName = name;
        argumentsDescription = args;
    }

    public String getCommandName() {
        return commandName;
    }

    /**
     * Executes command synchronously: i.e. current thread waits
     * until spawned process is finished
     * @return success of execution
     */
    public ToolCommandStatus runSync() {
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

    /**
     * Executes command asynchronously: i.e. current thread doesn't wait
     * until spawned process is finished
     * @return success of execution
     */
    public Pair<Process, ToolCommandStatus> runAsync() {
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

    private Pair<ProcessBuilder, ToolCommandStatus> prepareCommand() {
        Path tool = FileResolver.findFile(commandName);

        if (tool == null) {
            return new Pair<>(null, ToolCommandStatus.TOOL_NOT_FOUND);
        }

        List<String> commandList = null;
        if (registeredCommands.contains(commandName)) {
            commandList = new LinkedList<>(Arrays.asList("java", "-jar", tool.toString()));
        } else {
            commandList = new LinkedList<>();
            commandList.add(commandName);
        }

        commandList.addAll(argumentsDescription);

        String[] commandArray = new String[commandList.size()];
        ProcessBuilder pb = new ProcessBuilder(commandList.toArray(commandArray));

        Map<String, String> env = pb.environment();
        env.clear();
        env.putAll(Environment.getInstance().getEnvironmentMap());

        return new Pair<>(pb, ToolCommandStatus.SUCCESS);
    }
}
