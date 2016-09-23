package au.sdshell.driver;

import au.sdshell.common.Environment;

/**
 * Created by andy on 9/23/16.
 */
public class MessageWriter {
    public void showGreeting() {
        System.out.print(Environment.getInstance().getCurrentDir() + "$ ");
    }

    public void onFailedToExecute(String commandName) {
        System.out.println("failed to execute: " + commandName);
    }

    public void onToolNotFound(String commandName) {
        System.out.println("command not found: " + commandName);
    }

    public void onUnpipableCommand() {
        System.out.println("Cannot pipe such type of commands");
    }
}
