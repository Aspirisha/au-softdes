package au.sdshell.driver.command;

import au.sdshell.common.Environment;
import au.sdshell.common.FileResolver;

import java.io.*;
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

    public class ToolCommandResult {
        public final ToolCommandStatus status;
        public BufferedReader commandOutput;

        ToolCommandResult(ToolCommandStatus s, BufferedReader br) {
            status = s;
            commandOutput = br;
        }
    }

    public ToolCommand(String name, List<String> args, boolean isPiped) {
        commandName = name;
        argumentsDescription = args;
        this.isPiped = isPiped;
    }

    public String getCommandName() {
        return commandName;
    }

    public ToolCommandResult run(BufferedReader br) {
        Process theProcess = null;

        commandName = Environment.substituteVariables(commandName);
        Path tool = FileResolver.findFile(commandName);

        if (tool == null) {
            return new ToolCommandResult(ToolCommandStatus.TOOL_NOT_FOUND, null);
        }

        List<String> commandList = new LinkedList<>(Arrays.asList("java", "-jar", tool.toString()));
        commandList.addAll(argumentsDescription
                .stream()
                .map(Environment::substituteVariables)
                .collect(Collectors.toList()));

        String[] commandArray = new String[commandList.size()];
        try {
            ProcessBuilder pb = new ProcessBuilder(commandList.toArray(commandArray));

            Map<String, String> env = pb.environment();
            env.clear();
            env.putAll(Environment.getInstance().getEnvironmentMap());
            pb.inheritIO();
            theProcess = pb.start();
            theProcess.waitFor();
        } catch (IOException e) {
            return new ToolCommandResult(ToolCommandStatus.EXECUTION_FAILURE, null);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        // read from the called program's standard output stream
        BufferedReader inStream = new BufferedReader(new InputStreamReader
                (theProcess.getInputStream()));
        if (br != null) {
            BufferedWriter bufferedWriter = new BufferedWriter(new OutputStreamWriter(theProcess.getOutputStream()));
            String lineToPipe;
            try {
                while ((lineToPipe = br.readLine()) != null) {
                    System.out.println("Output process1 / Input process2:" + lineToPipe);

                    bufferedWriter.write(lineToPipe + '\n');
                    bufferedWriter.flush();

                }
            } catch (IOException e) {
                e.printStackTrace();
            }

        }

        if (!isPiped) {
            inStream.lines().forEach(System.out::println);
            return new ToolCommandResult(ToolCommandStatus.SUCCESS, null);
        } else {
            return new ToolCommandResult(ToolCommandStatus.SUCCESS, inStream);
        }

    }
}
