package au.sdshell.driver;

import au.sdshell.driver.command.AssignCommand;
import au.sdshell.driver.command.ExitCommand;
import au.sdshell.driver.command.ToolCommand;
import org.apache.tools.ant.types.Commandline;

import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 * Created by andy on 9/17/16.
 */
public class InputParser {
    static List<Object> parseInput(String commandString) {
        List<Object> commands = new ArrayList<>();

        Commandline cmdl = new Commandline(commandString);
        String command = cmdl.getExecutable();

        if (null == command) {
            return commands;
        }

        Pattern p = Pattern.compile("^(\\w+)=(\\w+)$");
        Matcher m = p.matcher(command);
        if (m.find()) {
            commands.add(new AssignCommand(m.group(1), m.group(2)));
            return commands;
        }

        for (Iterator<Commandline.Argument> it = cmdl.iterator(); command != null; ) {
            if (command.equals("exit")) {
                commands.add(new ExitCommand());
                return commands;
            }

            List<String> args = new LinkedList<>();
            boolean finishedCommand = false;
            for (String arg = getNextArgumentAndPutIntoQuotesIfNeeded(it);
                 arg != null;
                 arg = getNextArgumentAndPutIntoQuotesIfNeeded(it)) {

                if (arg.equals("|")) {
                    commands.add(new ToolCommand(command, args, true));
                    finishedCommand = true;
                    break;
                }
                args.add(arg);
            }
            if (!finishedCommand) {
                commands.add(new ToolCommand(command, args, false));
                break;
            } else {
                command = getNextArgumentAndPutIntoQuotesIfNeeded(it);
            }
        }
        return commands;
    }

    static private String getNextArgumentAndPutIntoQuotesIfNeeded(Iterator<Commandline.Argument> it) {
        if (!it.hasNext())
            return null;

        String arg = Arrays.stream(it.next().getParts()).collect(Collectors.joining(" "));
        if (arg.contains(" ")) {
            arg = String.format("\"%s\"", arg);
        }

        return arg;
    }
}
