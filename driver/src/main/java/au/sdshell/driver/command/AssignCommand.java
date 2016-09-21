package au.sdshell.driver.command;

import au.sdshell.common.Environment;

/**
 * Created by andy on 9/18/16.
 */
public class AssignCommand {
    String name;
    String value;

    public AssignCommand(String name, String value) {
        this.name = name;
        this.value = value;
    }

    public void run() {
        String realValue = Environment.substituteVariables(value);
        Environment.getInstance().setVariable(name, realValue);
    }
}
