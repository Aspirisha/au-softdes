package au.sdshell.tools.ls;

import au.sdshell.common.Environment;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

/**
 * Created by: Egor Gorbunov
 * Date: 12/8/16
 * Email: egor-mailbox@ya.com
 */
public class LSTool {
    /**
     * Prints contents of directory stored in PWD env. var;
     */
    public static void main(String[] args) {
        Path pwd = Paths.get(Environment.getInstance().getVariable("PWD"));
        try {
            Files.list(pwd).forEach((p) -> {
                if (Files.isDirectory(p)) {
                    System.out.print("d ");
                } else {
                    System.out.print("f ");
                }
                System.out.println(pwd.relativize(p).normalize());
            });
        } catch (IOException e) {
            System.err.println("Error reading directory contents: " + e.getMessage());
        }
    }
}
