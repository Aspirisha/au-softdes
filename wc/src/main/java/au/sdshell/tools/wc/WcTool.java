package au.sdshell.tools.wc;

import au.sdshell.common.FileResolver;
import se.softhouse.jargo.Argument;
import se.softhouse.jargo.CommandLineParser;
import se.softhouse.jargo.ParsedArguments;

import java.io.*;
import java.nio.file.Path;
import java.util.Scanner;
import java.util.StringTokenizer;

import static se.softhouse.jargo.Arguments.fileArgument;

/**
 * Counts words, bytes and lines in passed file (if given) or in standard input.
 * Created by andy on 9/22/16.
 */
public class WcTool {
    private Argument<File> fileNameArg = fileArgument()
            .description("path to file to show contents").defaultValue(null).build();
    private ParsedArguments arguments = null;
    private File fileName = null;

    private WcTool(String[] args) {
        try {
            arguments = CommandLineParser
                    .withArguments(fileNameArg)
                    .parse(args);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void runCommand() {
        if (arguments != null) {
            fileName = arguments.get(fileNameArg);
        }

        int linesNumber = 0;
        int wordsNumber = 0;
        if (fileName != null) {
            Path filePath = FileResolver.findFile(fileName.toString());
            if (filePath == null) {
                onFileNotFound();
                return;
            }

            long bytesNumber = fileName.length();
            try (BufferedReader br = new BufferedReader(new FileReader(filePath.toString()))) {
                String line;
                while ((line = br.readLine()) != null) {
                    linesNumber++;
                    wordsNumber += new StringTokenizer(line, " ,").countTokens();
                }
            } catch (FileNotFoundException e) {
                onFileNotFound();
                return;
            } catch (IOException e) {
                e.printStackTrace();
                return;
            }

            System.out.println(String.format("\t%d\t%d\t%d\t%s", linesNumber,
                    wordsNumber, bytesNumber, fileName.toString()));
        } else {
            Scanner sc = new Scanner(System.in);
            int bytesNumber = 0;
            while (sc.hasNextLine()) {
                String line = sc.nextLine();
                linesNumber++;
                wordsNumber += new StringTokenizer(line, " ,").countTokens();
                bytesNumber += line.length() + 1;
            }

            System.out.println(String.format("\t%d\t%d\t%d", linesNumber,
                    wordsNumber, bytesNumber));
        }

    }

    private void onFileNotFound() {
        System.out.println("wc: " + fileName + ": No such file or directory");
    }

    public static void main(String[] args) {
        WcTool runner = new WcTool(args);
        runner.runCommand();
    }
}
