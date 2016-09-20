package au.sdshell.driver;

import org.junit.Test;

import java.io.File;
import java.nio.file.Path;
import java.nio.file.Paths;

import static junit.framework.TestCase.assertTrue;

/**
 * Created by andy on 9/19/16.
 */
public class ToolResolverTest {

    @Test
    public void resolveToolFromPath() {
        final String toolName = "findme";
        String path = Environment.getInstance().getVariable("PATH");
        Path testPath = Paths.get(System.getProperty("user.dir"), "src", "test", "resources");
        path += File.pathSeparator + testPath;

        Environment.getInstance().setVariable("PATH", path);

        System.out.println(path);

        Path p = ToolResolver.findTool(toolName);
        assertTrue(p.toString().length() > 0);
        assertTrue(p.getFileName().toString().equals(toolName));
    }
}
