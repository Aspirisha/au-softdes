package au.sdshell.tools.grep;

import au.sdshell.common.FileResolver;
import com.google.common.collect.Range;
import se.softhouse.jargo.Argument;
import se.softhouse.jargo.ArgumentException;
import se.softhouse.jargo.CommandLineParser;
import se.softhouse.jargo.ParsedArguments;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.nio.file.Path;
import java.util.NoSuchElementException;
import java.util.Scanner;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import static se.softhouse.jargo.Arguments.*;

/**
 * Created by andy on 9/29/16.
 */
public class GrepTool {
    public static final String ANSI_RESET = "\u001B[0m";
    public static final String ANSI_RED = "\u001B[31m";
    public static final String ANSI_CYAN = "\u001B[36m";

    private boolean caseInsensitive;
    private boolean wordsOnly;
    private int linesAfterMatch;
    String pattern;
    String fileName;

    GrepTool(String[] args) {
        Argument<Boolean> caseInsensitive = optionArgument("-i")
                .description("Case insensitive search.")
                .defaultValue(false).build();
        Argument<Boolean> wordsOnly = optionArgument("-w")
                .description("A greeting phrase to greet new connections with.")
                .defaultValue(false).build();
        Argument<Integer> linesAfterMatch = integerArgument("-A")
                .defaultValue(0)
                .description("Number of lines to print after each line containing a match.")
                .metaDescription("<n>")
                .limitTo(Range.closed(0, Integer.MAX_VALUE))
                .build();
        Argument<String> pattern = stringArgument()
                .description("A regular expression that is searched.").required().build();
        Argument<String> fileName = stringArgument()
                .description("A file in which pattern is searched. " +
                        "If not specified, read standard input.").defaultValue("").build();

        try {
            ParsedArguments arguments = CommandLineParser.withArguments(caseInsensitive, wordsOnly, linesAfterMatch)
                    .andArguments(pattern, fileName).programName("grep").parse(args);

            this.caseInsensitive = arguments.get(caseInsensitive);
            this.wordsOnly = arguments.get(wordsOnly);
            this.linesAfterMatch = arguments.get(linesAfterMatch);
            this.pattern = arguments.get(pattern);
            this.fileName = arguments.get(fileName);
        } catch(ArgumentException exception) {
            System.out.println(exception.getMessageAndUsage());
            System.exit(1);
        }
    }

    private void runCommand() {
        if (wordsOnly) {
            pattern = String.format("%s%s%s", "\\b", pattern, "\\b");
        }
        if (caseInsensitive) {
            pattern = "(?i)" + pattern;
        }

        final Pattern p = Pattern.compile(pattern);
        if (!fileName.isEmpty()) {
            Path filePath = FileResolver.findFile(fileName);
            if (filePath == null) {
                onFileNotFound();
                return;
            }

            try (BufferedReader br = new BufferedReader(new FileReader(filePath.toString()))) {
                String line;
                int counter = linesAfterMatch;
                while ((line = br.readLine()) != null) {
                    boolean matches = processString(p, line);
                    if (!matches && counter > 0) {
                        System.out.println(line);
                        counter--;
                        if (counter == 0) {
                            System.out.println(ANSI_CYAN + "--" + ANSI_RESET);
                        }
                    } else if (matches) {
                        counter = linesAfterMatch;
                    }
                }
            } catch (FileNotFoundException e) {
                onFileNotFound();
            } catch (IOException e) {
                e.printStackTrace();
            }
        } else {
            Scanner sc = new Scanner(System.in);
            for (;;) {
                try {
                    String s = sc.nextLine();
                    if (!processString(p, s)) {
                        System.out.println(s);
                    }
                } catch (NoSuchElementException e) {
                    return;
                }
            }
        }

    }

    private boolean processString(Pattern p, String s) {
        Matcher m = p.matcher(s);
        int noMatchStart = 0;
        if (!m.find())
            return false;

        do {
            System.out.print(s.substring(noMatchStart, m.start()));
            String match = m.group();
            System.out.print(ANSI_RED + match + ANSI_RESET);
            noMatchStart = m.end();
        } while (m.find());

        System.out.println(s.substring(noMatchStart));

        return true;
    }

    private void onFileNotFound() {
        System.out.println("grep: " + fileName + ": No such file or directory");
    }

    public static void main(String[] args) {
        GrepTool runner = new GrepTool(args);

        runner.runCommand();
    }
}
