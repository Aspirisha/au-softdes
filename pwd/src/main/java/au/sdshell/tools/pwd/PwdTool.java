package au.sdshell.tools.pwd;

import au.sdshell.common.Environment;

/**
 * Created by andy on 9/21/16.
 */
public class PwdTool {
    public static void main(String[] args) {
        System.out.println(Environment.getInstance().getVariable("PWD"));
    }
}
