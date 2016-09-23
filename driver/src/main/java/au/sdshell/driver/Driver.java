package au.sdshell.driver;

import au.sdshell.common.Environment;
import au.sdshell.driver.command.AssignCommand;
import au.sdshell.driver.command.ExitCommand;
import au.sdshell.driver.command.ToolCommand;
import javafx.util.Pair;
import sun.misc.Signal;
import sun.misc.SignalHandler;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;
import java.util.List;
import java.util.Scanner;
import java.util.stream.Collectors;

import static au.sdshell.driver.command.ToolCommand.ToolCommandStatus.TOOL_NOT_FOUND;

/**
 * Created by andy on 9/15/16.
 */
public class Driver {
    Signal sigInt = new Signal("INT");
    // First register with SIG_DFL, just to get the old handler.
    final SignalHandler oldHandler = Signal.handle(sigInt, SignalHandler.SIG_DFL);
    volatile boolean isWaitingForChildProcess = false;
    private MessageWriter messageWriter;

    private void setUpSignalHandling() {
        // Now register the actual handler
        Signal.handle(sigInt, signal -> {
            if (!isWaitingForChildProcess)
                oldHandler.handle(signal);
        });
    }

    private void setUpEnvironment() {
        String defaultPath = Arrays.asList(System.getProperty("user.home"),
                "sdshell", "tools").stream().collect(Collectors.joining(File.separator));

        Environment env = Environment.getInstance();
        env.clearVariables();
        env.setVariable("PATH", defaultPath);
        env.setVariable("PWD", System.getProperty("user.dir"));
    }

    public Driver() {
        messageWriter = new MessageWriter();
        setUpEnvironment();
        setUpSignalHandling();
    }

    public void run() {
        Scanner terminalInput = new Scanner(System.in);
        while (true) {
            messageWriter.showGreeting();
            String s = terminalInput.nextLine();
            List<Object> commands = InputParser.parseInput(s);

            if (commands.size() == 1) {
                runSingleCommand(commands.get(0));
            } else {
                runPipedToolCommands(commands);
            }
        }
    }

    private void runSingleCommand(Object c) {
        if (c instanceof ExitCommand) {
            ((ExitCommand) c).run();
        } else if (c instanceof AssignCommand) {
            ((AssignCommand) c).run();
        } else {
            runToolCommand((ToolCommand) c);
        }
    }

    private void runToolCommand(ToolCommand c) {
        isWaitingForChildProcess = true;
        ToolCommand.ToolCommandStatus status = c.run();
        isWaitingForChildProcess = false;
        onProcessStatusAppeared(status, c.getCommandName());
    }

    private boolean onProcessStatusAppeared(ToolCommand.ToolCommandStatus status, String commandName) {
        switch (status) {
            case EXECUTION_FAILURE:
                messageWriter.onFailedToExecute(commandName);
                return false;
            case TOOL_NOT_FOUND:
                messageWriter.onToolNotFound(commandName);
                return false;
            default:
                return true;
        }
    }

    private void runPipedToolCommands(List<Object> commands) {
        for (Object c : commands) {
            if (!(c instanceof ToolCommand)) {
                messageWriter.onUnpipableCommand();
                return;
            }
        }

        List<Pair<Process, ToolCommand.ToolCommandStatus>> proc = commands.stream()
                .map(c -> ((ToolCommand)c).prepareCommandForPiping())
                .collect(Collectors.toList());

        boolean noErrors = true;
        for (int i = 0; i < proc.size(); i++) {
            noErrors &= onProcessStatusAppeared(proc.get(i).getValue(),
                    ((ToolCommand) commands.get(i)).getCommandName());
        }

        if (!noErrors) {
            return;
        }

        InputStream in = null;
        try {
            in = Piper.pipe(proc
                    .stream()
                    .map(Pair::getKey)
                    .collect(Collectors.toList()));
        } catch (InterruptedException e) {
            return;
        }

        // Show output of last process
        java.io.BufferedReader r = new java.io.BufferedReader(new java.io.InputStreamReader(in));
        String s = null;
        try {
            while ((s = r.readLine()) != null) {
                System.out.println(s);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] a) {
        Driver d = new Driver();
        d.run();
    }
}
