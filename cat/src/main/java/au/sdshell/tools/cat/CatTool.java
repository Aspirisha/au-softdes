package au.sdshell.tools.cat;

import se.softhouse.jargo.Argument;
import se.softhouse.jargo.ArgumentException;
import se.softhouse.jargo.CommandLineParser;
import se.softhouse.jargo.ParsedArguments;

import static se.softhouse.jargo.Arguments.*;

/**
 * Created by andy on 9/19/16.
 */
public class CatTool {
    public static void main(String[] args) {
        Argument<?> helpArgument = helpArgument("-h", "--help"); //Will throw when -h is encountered
        Argument<Boolean> linesNumber = optionArgument("-n", "--number")
                .description("number all output lines").build();

        try {
            ParsedArguments arguments = CommandLineParser.withArguments(linesNumber)
                    .andArguments(helpArgument)
                    .parse(args);

            System.out.println("number lines: " + arguments.get(linesNumber));
        } catch (ArgumentException exception) {
            System.out.println(exception.getMessageAndUsage());
            System.exit(1);
        }
    }
}
