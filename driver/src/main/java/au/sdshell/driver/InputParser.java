package au.sdshell.driver;

import au.sdshell.common.Environment;
import au.sdshell.driver.command.AssignCommand;
import au.sdshell.driver.command.ExitCommand;
import au.sdshell.driver.command.ToolCommand;
import edu.rice.cs.util.ArgumentTokenizer;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by andy on 9/17/16.
 */
public class InputParser {
    /**
     * Parses user input string into list of commands
     * The list contains n + 1 commands, where n is the number of pipe characters,
     * found in input.
     * @param commandString string that user entered as input
     * @return list of commands. Every entity in list is instance of one of classes,
     *          implemented in au.sdshell.driver.command
     */
    public static List<Object> parseInput(String commandString) {
        List<Object> commands = new ArrayList<>();

        commandString = Environment.substituteVariables(commandString);

        List<String> args = ArgumentTokenizer.tokenize(commandString);

        if (args.isEmpty())
            return commands;
        Pattern p = Pattern.compile("^(\\w+)=(\\w+)$");
        Matcher m = p.matcher(args.get(0));
        if (m.find()) {
            commands.add(new AssignCommand(m.group(1), m.group(2)));
            return commands;
        }

        String command = null;
        LinkedList<String> currentArgs = new LinkedList<>();
        for (String s : args) {
            if (command == null) {
                command = s;
                if (command.equals("exit")) {
                    commands.add(new ExitCommand());
                    return commands;
                }
                continue;
            }

            if (s.equals("|")) {
                commands.add(new ToolCommand(command, currentArgs));
                command = null;
                currentArgs = new LinkedList<>();
                continue;
            }

            currentArgs.add(s);
        }

        if (command != null) {
            commands.add(new ToolCommand(command, currentArgs));
        }
        return commands;
    }
}
