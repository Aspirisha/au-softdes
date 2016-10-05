package au.sdshell.tools.cat;

import au.sdshell.common.FileResolver;
import se.softhouse.jargo.Argument;
import se.softhouse.jargo.CommandLineParser;
import se.softhouse.jargo.ParsedArguments;

import java.io.*;
import java.nio.file.Path;
import java.util.NoSuchElementException;
import java.util.Scanner;

import static se.softhouse.jargo.Arguments.fileArgument;
import static se.softhouse.jargo.Arguments.optionArgument;

/**
 * Tool that can show either given file content (if file name is passed as
 * input agument) or reprint stdin lines to stdout, if no file specified.
 * If flag -n is passed, each output line is enumerated.
 * Created by andy on 9/19/16.
 */
public class CatTool {
    private Argument<Boolean> linesNumber = optionArgument("-n", "--number")
            .description("number all output lines").defaultValue(false).build();

    private Argument<File> fileNameArg = fileArgument().description("path to file to show contents").defaultValue(null).build();
    private ParsedArguments arguments = null;
    private int counter = 1;

    private Boolean needNumbers = false;
    private File fileName = null;

    private CatTool(String[] args) {
        CommandLineParser parser = CommandLineParser
                .withArguments(linesNumber, fileNameArg);
        try {
            arguments = parser.parse(args);
        } catch (Exception e) {
            System.out.println("Unexpected arguments.");
            parser.usage().printOn(System.out);
            System.exit(0);
        }
    }

    private void runCommand() {
        if (arguments != null) {
            needNumbers = arguments.get(linesNumber);
            fileName = arguments.get(fileNameArg);
        }

        if (fileName != null) {
            Path filePath = FileResolver.findFile(fileName.toString());
            if (filePath == null) {
                onFileNotFound();
                return;
            }

            try (BufferedReader br = new BufferedReader(new FileReader(filePath.toString()))) {
                String line;
                while ((line = br.readLine()) != null) {
                    printLine(line);
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
                    System.out.println(sc.nextLine());
                } catch (NoSuchElementException e) {
                    return;
                }
            }
        }
    }

    private void onFileNotFound() {
        System.out.println("cat: " + fileName + ": No such file or directory");
    }

    /**
     * entry point for cat tool
     * @param args command line arguments
     */
    public static void main(String[] args) {
        CatTool runner = new CatTool(args);
        runner.runCommand();
    }

    private void printLine(String s) {
        if (needNumbers) {
            System.out.println("\t " + counter + "  " + s);
            counter++;
        } else {
            System.out.println(s);
        }
    }
}
