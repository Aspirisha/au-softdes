package au.sdshell.driver;

import java.io.BufferedReader;

/**
 * Created by andy on 9/18/16.
 */
public class AssignCommand implements Command {
    String name;
    String value;

    AssignCommand(String name, String value) {
        this.name = name;
        this.value = value;
    }

    @Override
    public BufferedReader run(BufferedReader unused) {
        String realValue = Environment.substituteVariables(value);
        Environment.getInstance().setVariable(name, realValue);

        return null;
    }
}
