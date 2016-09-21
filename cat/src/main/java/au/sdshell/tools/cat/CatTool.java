package au.sdshell.tools.cat;

import au.sdshell.common.FileResolver;
import javafx.util.Pair;
import se.softhouse.jargo.Argument;
import se.softhouse.jargo.ArgumentException;
import se.softhouse.jargo.CommandLineParser;
import se.softhouse.jargo.ParsedArguments;

import java.io.*;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.Map;
import java.util.Scanner;
import java.util.function.Function;

import static se.softhouse.jargo.Arguments.*;

/**
 * Created by andy on 9/19/16.
 */
public class CatTool {
    private String[] rawArgs;
    private Argument<?> helpArgument = helpArgument("-h", "--help"); //Will throw when -h is encountered
    private Argument<Boolean> linesNumber = optionArgument("-n", "--number")
            .description("number all output lines").defaultValue(false).build();

    private Argument<File> fileNameArg = fileArgument().description("path to file to show contents").defaultValue(null).build();
    private ParsedArguments arguments = null;
    private int counter = 1;

    private Boolean needNumbers = false;
    private File fileName = null;

    CatTool(String[] args) {
        rawArgs = args;

        try {
            arguments = CommandLineParser
                    .withArguments(linesNumber, fileNameArg)
                    .andArguments(helpArgument)
                    .parse(args);
        } catch (Exception e) {
            e.printStackTrace();
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
                System.out.println(sc.next());
            }
        }

    }

    private void onFileNotFound() {
        System.out.println("cat: " + fileName + ": No such file or directory");
    }

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
