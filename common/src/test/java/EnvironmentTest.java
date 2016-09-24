import au.sdshell.common.Environment;
import org.junit.Test;

import java.util.regex.Pattern;

import static junit.framework.TestCase.assertTrue;

/**
 * Created by andy on 9/22/16.
 */
public class EnvironmentTest {

    @Test
    public void testVariablesSubstitution() {
        Environment env = Environment.getInstance();

        env.setVariable("PATH", "word");
        String s1 = "The Path is \"$PATH\"";
        String s2 = "The Path is not '$PATH'";
        assertTrue(Environment.substituteVariables(s2).equals(s2));
        assertTrue(Environment.substituteVariables(s1).equals(s1.replaceAll(Pattern.quote("$PATH"), "word")));
    }



}
