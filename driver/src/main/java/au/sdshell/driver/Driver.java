package au.sdshell.driver;

import java.io.BufferedReader;
import java.util.List;
import java.util.Scanner;

/**
 * Created by andy on 9/15/16.
 */
public class Driver {
    public static void main(String[] a) {
        Scanner terminalInput = new Scanner(System.in);
        while (true) {
            System.out.print("> ");
            String s = terminalInput.nextLine();
            List<Command> result = InputParser.parseInput(s);

            BufferedReader br = null;
            for (Command c : result) {
                br = c.run(br);
            }
        }
    }
}
