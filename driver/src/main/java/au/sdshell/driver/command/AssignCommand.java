package au.sdshell.driver.command;

import au.sdshell.common.Environment;

/**
 * Represents environment variable assign command
 * Created by andy on 9/18/16.
 */
public class AssignCommand {
    final String name;
    final String value;

    public AssignCommand(String name, String value) {
        this.name = name;
        this.value = value;
    }

    /**
     * Puts variable with given name and value into environment
     */
    public void run() {
        String realValue = Environment.substituteVariables(value);
        Environment.getInstance().setVariable(name, realValue);
    }
}
