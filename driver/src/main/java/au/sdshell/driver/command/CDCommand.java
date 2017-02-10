package au.sdshell.driver.command;

import au.sdshell.common.Environment;
import au.sdshell.common.FileResolver;

import java.nio.file.Files;
import java.nio.file.Path;

/**
 * Shell command, which changes current working directory, which is passed
 * as first argument
 */
public class CDCommand {
    private final String distUserDir;

    public CDCommand(String distUserDir) {
        this.distUserDir = distUserDir;
    }

    public void run() {
        String newDistDir = Environment.substituteVariables(distUserDir);
        Environment env = Environment.getInstance();
        Path newPWD = env.getCurrentDir().resolve(newDistDir).normalize();
        if (!Files.exists(newPWD) || !Files.isDirectory(newPWD)) {
            System.out.println("No such directory");
        } else {
            env.setCurrentDir(newPWD);
        }
    }
}
