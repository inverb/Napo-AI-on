# How to run benchmarks

0. Pull cg-brutaltester and referee repositories:
```
git submodule update --init
```

1. Build jar for the cg-brutaltestser and copy it to benchmarks' root:
(Maven is required to be installed, probably can do away by using Docker)
```
cd cg-brutaltester
mvn package
cp target/cg-brutaltester-1.0.0-SNAPSHOT.jar ../cg-brutaltester.jar
cd ..
```

2. Build jar for the referee and copy it to benchmarks' root.
We also apply a patch that makes the referee work with brutaltester which outputs a "###Seed" line to referee, which the referee doesn't expect...
(you _might_ nned to have Kotlin installed also... idk)
```
cd cg-referee-ghost-in-the-cell
git apply ../referee.patch
./gradlew jar
cp build/libs/cg-referee-ghost-in-the-cell.jar ../referee.jar
cd ..
```

This above may work but 99% will complain when calling `java -jar referee.jar` about missing manifest or something. I fixed that by adding the following to `build.gradle.kts`:
```
// Added because I wasn't able to generate the jar otherwise
tasks.withType<Jar> {
    manifest {
        attributes["Main-Class"] = "Referee"
    }
}
```

The above might not work because the author didn't include some important Gradle-related files. One method is to call `gradle wrapper` if you have _exactly_ Gradle 5.6.3 installed. Another option is to call (within `cg-referee-ghost-in-the-cell` directory):
```
docker run -it -v `pwd`:/ws -w /ws gradle:5.6.3-jdk8 gradle wrapper
sudo chown -R $USER:$USER .
```
before calling `./gradlew jar`. This way we generate the wrapper via Docker instead of installing Gradle directly on our machine.

3. Build agents:
```
cd ../agents
make
cd ../benchmarks
```

4. Compare agents:
```
java -jar cg-brutaltester.jar -o -r "java -jar referee.jar" -i 42 -p1 ../agents/artifacts/basic -p2 ../agents/artifacts/basic -v
```
(remove `-v` to see less output)

Known/suspected issues:
- couldn't generate jar for the referee, added following lines to `build.gradle.kts`:
```
// Added because I wasn't able to generate the jar otherwise
tasks.withType<Jar> {
    manifest {
        attributes["Main-Class"] = "Referee"
    }
}
```
- despite setting the seed to 42, referee doesn't generate the same input time
- referee complains about invalid action:
```
10:56:48,175 DEBUG [com.magusgeek.brutaltester.OldGameThread] [Game 1] Player 0: MOVE 1 0 1;MOVE 1 3 1;MOVE 1 4 1;MOVE 1 5 6;MOVE 1 6 6;MOVE 1 7 8;MOVE 1 8 8;MOVE 1 9 1;MOVE 1 10 1
10:56:48,189 DEBUG [com.magusgeek.brutaltester.OldGameThread] [Game 1] Referee error: ###Error 0 InvalidInput Invalid action : ###Start 2
```
the "###" lines are for cg-brutaltester <-> referee communication :thinking_face: