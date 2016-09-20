package au.sdshell.driver;

import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

import org.apache.tools.ant.types.Commandline;

import static java.util.Arrays.stream;
import static org.apache.tools.ant.types.Commandline.translateCommandline;

/**
 * Created by andy on 9/17/16.
 */
public class InputParser {
    static List<Command> parseInput(String commandString) {
        List<Command> commands = new ArrayList<>();

        Commandline cmdl = new Commandline(commandString);
        String command = cmdl.getExecutable();
        Pattern p = Pattern.compile("^(\\w+)=(\\w+)$");
        Matcher m = p.matcher(command);
        if (m.find()) {
            commands.add(new AssignCommand(m.group(1), m.group(2)));
            return commands;
        }

        for (Iterator<Commandline.Argument> it = cmdl.iterator(); it.hasNext(); ) {
            if (command.matches("exit")) {
                commands.add(new ExitCommand());
                return commands;
            }

            List<String> args = new LinkedList<>();
            boolean finishedCommand = false;
            while (it.hasNext()) {
                String arg = Arrays.stream(it.next().getParts()).collect(Collectors.joining(" "));
                if (arg.contains(" ")) {
                    arg = String.format("\"%s\"", arg);
                }

                if (arg.equals("|")) {
                    commands.add(new ToolCommand(command, args, true));
                    finishedCommand = true;
                    break;
                }
                args.add(arg);
            }
            if (!finishedCommand) {
                commands.add(new ToolCommand(command, args, false));
            }
        }
        return commands;
    }
}
