package au.sdshell.driver;

import au.sdshell.common.Environment;
import au.sdshell.driver.command.AssignCommand;
import au.sdshell.driver.command.ExitCommand;
import au.sdshell.driver.command.ToolCommand;

import java.io.BufferedReader;
import java.io.File;
import java.util.Arrays;
import java.util.List;
import java.util.Scanner;
import java.util.stream.Collectors;

/**
 * Created by andy on 9/15/16.
 */
public class Driver {
    public static void main(String[] a) {
        Scanner terminalInput = new Scanner(System.in);
        String defaultPath = Arrays.asList(System.getProperty("user.home"),
                "sdshell", "tools").stream().collect(Collectors.joining(File.separator));

        Environment env = Environment.getInstance();
        env.clearVariables();
        env.setVariable("PATH", defaultPath);
        env.setVariable("PWD", System.getProperty("user.dir"));

        while (true) {
            System.out.print(Environment.getInstance().getCurrentDir() + "$ ");
            String s = terminalInput.nextLine();
            List<Object> commands = InputParser.parseInput(s);

            BufferedReader br = null;
            for (Object c : commands) {
                if (c instanceof ExitCommand) {
                    ((ExitCommand) c).run();
                } else if (c instanceof AssignCommand) {
                    ((AssignCommand) c).run();
                } else {
                    ToolCommand.ToolCommandResult result = ((ToolCommand) c).run(br);
                    switch (result.status) {
                        case EXECUTION_FAILURE:
                            System.out.println("failed to execute: " + ((ToolCommand) c).getCommandName());
                            break;
                        case TOOL_NOT_FOUND:
                            System.out.println("command not found: " + ((ToolCommand) c).getCommandName());
                            break;
                    }

                    br = result.commandOutput;
                }
            }
        }
    }
}
