package au.sdshell.driver;

import org.junit.Assert;
import org.junit.Test;

import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

import static au.sdshell.driver.InputParser.parseInput;
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
        List<Command> commands = InputParser.parseInput(s);

        assertNotNull(commands);
        assertTrue(commands.size() == 1);
        assertTrue(commands.get(0) instanceof ToolCommand);
        ToolCommand cmd = (ToolCommand) commands.get(0);

        assertFalse(cmd.isPiped);
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
        List<Command> commands = InputParser.parseInput(fullCommand);

        Assert.assertNotNull(commands);
        assertEquals(2, commands.size());
        assertTrue(commands.get(0) instanceof ToolCommand && commands.get(1) instanceof ToolCommand);
        ToolCommand tc1 = (ToolCommand) commands.get(0);
        ToolCommand tc2 = (ToolCommand) commands.get(1);
        assertTrue(tc1.isPiped);
        assertFalse(tc2.isPiped);
    }

    @Test
    public void parseAssignmentSimpleTest() {
        String s = "foo=bar";

        List<Command> commands = InputParser.parseInput(s);
        assertNotNull(commands);
        assertEquals(1, commands.size());
        assertTrue(commands.get(0) instanceof AssignCommand);

        AssignCommand ac = (AssignCommand) commands.get(0);

        assertEquals("foo", ac.name);
        assertEquals("bar", ac.value);
    }
}
