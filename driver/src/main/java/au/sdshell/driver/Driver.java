package au.sdshell.driver;

import au.sdshell.common.Environment;
import au.sdshell.driver.command.AssignCommand;
import au.sdshell.driver.command.CDCommand;
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
import java.util.NoSuchElementException;
import java.util.Scanner;
import java.util.stream.Collectors;

/**
 * Created by andy on 9/15/16.
 */
public class Driver {
    public static final String rootDirectory =  Arrays.asList(System.getProperty("user.home"),
            "sdshell").stream().collect(Collectors.joining(File.separator));

    private Signal sigInt = new Signal("INT");
    private final SignalHandler oldHandler = Signal.handle(sigInt, SignalHandler.SIG_DFL);
    private volatile boolean isWaitingForChildProcess = false;
    private MessageWriter messageWriter;

    private void setUpSignalHandling() {
        Signal.handle(sigInt, signal -> {
            if (!isWaitingForChildProcess)
                oldHandler.handle(signal);
        });
    }

    private void setUpEnvironment() {
        String defaultPath = Arrays.asList(rootDirectory, "tools")
                .stream().collect(Collectors.joining(File.separator));

        Environment env = Environment.getInstance();
        String path = env.getVariable("PATH");
        path = String.join(File.pathSeparator, defaultPath, path);
        env.setVariable("PATH", path);
    }

    private Driver() {
        messageWriter = new MessageWriter();
        setUpEnvironment();
        setUpSignalHandling();
    }

    private void run() {
        Scanner terminalInput = new Scanner(System.in);
        while (true) {
            messageWriter.showGreeting();
            String s = null;
            try {
                s = terminalInput.nextLine();
            } catch (NoSuchElementException e) { // e.g. in case of EOF signal
                return;
            }

            List<Object> commands = InputParser.parseInput(s);

            if (commands.size() == 0) {
                continue;
            }

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
        } else if (c instanceof CDCommand) {
            ((CDCommand) c).run();
        } else {
            runToolCommand((ToolCommand) c);
        }
    }

    private void runToolCommand(ToolCommand c) {
        isWaitingForChildProcess = true;
        ToolCommand.ToolCommandStatus status = c.runSync();
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
                .map(c -> ((ToolCommand)c).runAsync())
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
