package au.sdshell.driver.command;

import au.sdshell.driver.InputParser;
import au.sdshell.driver.command.AssignCommand;
import au.sdshell.driver.command.ToolCommand;
import org.junit.Assert;
import org.junit.Test;

import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

import static junit.framework.TestCase.assertNotNull;
import static junit.framework.TestCase.assertTrue;
import static org.hamcrest.CoreMatchers.is;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertThat;

/**
 * Created by andy on 9/18/16.
 */
public class InputParserTest {

    private String getCommandLineFromArgsAndCommand(String c, List<String> a) {
        return String.join(" ", c, a.stream().collect(Collectors.joining(" ")));
    }

    @Test
    public void parseSingleCommandInput() {
        List<String> args = Arrays.asList("\"hello world\"", "-b");
        String command = "cat";

        String s = getCommandLineFromArgsAndCommand(command, args);
        List<Object> commands = InputParser.parseInput(s);

        assertNotNull(commands);
        assertTrue(commands.size() == 1);
        assertTrue(commands.get(0) instanceof ToolCommand);
        ToolCommand cmd = (ToolCommand) commands.get(0);

        assertThat(cmd.argumentsDescription, is(args));
    }

    @Test
    public void parsePipedCommands() {
        List<String> args1 = Arrays.asList("\"hello world\"", "-b");
        String command1 = "cat";

        List<String> args2 = Arrays.asList("\"hello world\"", "-b");
        String command2 = "echo";

        String s1 = getCommandLineFromArgsAndCommand(command1, args1);
        String s2 = getCommandLineFromArgsAndCommand(command2, args2);
        String fullCommand = s1 + " | " + s2;
        List<Object> commands = InputParser.parseInput(fullCommand);

        Assert.assertNotNull(commands);
        assertEquals(2, commands.size());
        assertTrue(commands.get(0) instanceof ToolCommand && commands.get(1) instanceof ToolCommand);
    }

    @Test
    public void parseAssignmentSimpleTest() {
        String s = "foo=bar";

        List<Object> commands = InputParser.parseInput(s);
        assertNotNull(commands);
        assertEquals(1, commands.size());
        assertTrue(commands.get(0) instanceof AssignCommand);

        AssignCommand ac = (AssignCommand) commands.get(0);

        assertEquals("foo", ac.name);
        assertEquals("bar", ac.value);
    }
}
