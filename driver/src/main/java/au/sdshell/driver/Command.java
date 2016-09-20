package au.sdshell.driver;

import java.io.BufferedReader;

/**
 * Created by andy on 9/18/16.
 */
public interface Command {
    BufferedReader run(BufferedReader br);
}
