package au.sdshell.tools.ls;

import au.sdshell.common.Environment;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

/**
 * Shell command, which is used to list directory contents.
 * If no arguments specified it lists working directory, otherwise it
 * lists sequentially all directories passed as arguments
 */
public class LSTool {
    public static void main(String[] args) {
        Path pwd = Paths.get(Environment.getInstance().getVariable("PWD"));

        List<Path> directoriesToList = new ArrayList<>();

        if (args.length == 0) {
            directoriesToList.add(pwd);
        }

        for (String arg : args) {
            Path dir = pwd.resolve(arg);
            if (!Files.exists(dir)) {
                System.err.println("Directory not found: " + dir);
                continue;
            }

            if (!Files.isDirectory(dir)) {
                System.err.println("Not directory: " + arg);
                continue;
            }
            directoriesToList.add(dir);
        }

        try {
            for (Path dir : directoriesToList) {
                if (directoriesToList.size() != 1) {
                    System.out.println(pwd.relativize(dir) + ":");
                }
                Files.list(dir).forEach((p) -> {
                    if (Files.isDirectory(p)) {
                        System.out.print("d ");
                    } else {
                        System.out.print("f ");
                    }
                    System.out.println(dir.relativize(p).normalize());
                });
            }
        } catch (IOException e) {
            System.err.println("Error reading directory contents: " + e.getMessage());
        }
    }


}
