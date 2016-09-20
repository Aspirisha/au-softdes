package au.sdshell.driver;

import java.io.*;
import java.nio.file.Path;
import java.util.List;
import java.util.stream.Collectors;

/**
 * Created by andy on 9/16/16.
 */
public class ToolCommand implements Command {
    String commandName;
    List<String> argumentsDescription; // each entry here is space-free string with substituted environment variables
    boolean isPiped;


    ToolCommand(String name, List<String> args, boolean isPiped) {
        commandName = name;
        argumentsDescription = args;
        this.isPiped = isPiped;
    }

    void appendArguments(List<String> args) {
        argumentsDescription.addAll(args);
    }

    @Override
    public BufferedReader run(BufferedReader br) {
        Process theProcess = null;

        commandName = Environment.substituteVariables(commandName);
        Path tool = ToolResolver.findTool(commandName);
        try {
            theProcess = Runtime.getRuntime().exec(
                    String.join(" ", tool.toString(), argumentsDescription
                            .stream()
                            .peek(Environment::substituteVariables)
                            .collect(Collectors.joining(" "))));
        } catch (IOException e) {
            System.err.println("Error on exec() method");
            e.printStackTrace();

            return null;
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
            return null;
        } else {
            return inStream;
        }

    }
}
