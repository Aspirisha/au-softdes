package au.sdshell.driver;

import java.io.BufferedReader;

/**
 * Created by andy on 9/18/16.
 */
public class ExitCommand implements Command {
    @Override
    public BufferedReader run(BufferedReader br) {
        System.exit(0);
        return null;
    }
}
