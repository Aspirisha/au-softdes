import au.sdshell.common.Environment;
import au.sdshell.common.FileResolver;
import org.junit.Test;

import java.io.File;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Random;

import static junit.framework.TestCase.assertTrue;

/**
 * Created by andy on 9/19/16.
 */
public class FileResolverTest {

    @Test
    public void resolveToolFromPath() {
        final String toolName = "findme";
        String path = Environment.getInstance().getVariable("PATH");
        Path testPath = Paths.get(System.getProperty("user.dir"), "src", "test", "resources");
        path += File.pathSeparator + testPath;

        Environment.getInstance().setVariable("PATH", path);

        System.out.println(path);

        Path p = FileResolver.findFile(toolName);
        assertTrue(p.toString().length() > 0);
        assertTrue(p.getFileName().toString().equals(toolName));

        String randomFile = "PROBABLY_FILE_WITH_SUCH_NAME_DOESNT_EXIST";
        p = FileResolver.findFile(toolName);

        if (p != null) {
            File f = new File(p.toString());
            assertTrue(f.exists());
        }
    }
}
