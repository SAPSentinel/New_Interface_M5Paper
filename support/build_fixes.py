"""
Windows build fix for large ESP32 projects.

The linker command line (~536 object files + flags) exceeds Windows'
32768-char CreateProcess limit at BOTH the g++ invocation AND the
g++ -> collect2 internal invocation.

Fix: archive all object files into a single .a using ar's MRI script
mode (reads from stdin, no command-line length limit), then link with
just the archive. This keeps the g++ command well under 32KB.
"""
import sys
Import("env")

if sys.platform == "win32":
    import os, subprocess, tempfile

    def long_link_action(source, target, env):
        """Archive object files then link with the short command."""
        build_dir = str(target[0].dir)

        # --- Step 1: Get AR executable ---
        ar_exe = env.subst("$AR")
        if not os.path.isabs(ar_exe):
            for p in env["ENV"].get("PATH", "").split(os.pathsep):
                candidate = os.path.join(p, ar_exe)
                if os.path.isfile(candidate):
                    ar_exe = candidate
                    break
                candidate2 = candidate + ".exe"
                if os.path.isfile(candidate2):
                    ar_exe = candidate2
                    break

        # --- Step 2: Archive all source .o files via MRI script ---
        obj_files = [str(s).replace("\\", "/") for s in source]
        archive_path = os.path.join(build_dir, "project_objs.a").replace("\\", "/")

        # Remove old archive
        if os.path.exists(archive_path):
            os.remove(archive_path)

        # Build MRI script (no command-line length limit)
        mri_lines = ["CREATE %s" % archive_path]
        for obj in obj_files:
            mri_lines.append("ADDMOD %s" % obj)
        mri_lines.append("SAVE")
        mri_lines.append("END")
        mri_script = "\n".join(mri_lines) + "\n"

        mri_path = os.path.join(build_dir, "ar_mri.txt")
        with open(mri_path, "w") as f:
            f.write(mri_script)

        print("  Archiving %d object files..." % len(obj_files))
        ar_result = subprocess.run(
            [ar_exe, "-M"],
            input=mri_script,
            text=True,
            cwd=env.Dir("#").abspath,
        )
        if ar_result.returncode != 0:
            print("  ERROR: ar failed with code %d" % ar_result.returncode)
            return ar_result.returncode

        if not os.path.exists(archive_path):
            print("  ERROR: archive not created at %s" % archive_path)
            return 1

        print("  Archive: %s (%d bytes)" % (archive_path,
                                              os.path.getsize(archive_path)))

        # --- Step 3: Build short link command ---
        link_exe = env.subst("$LINK")
        if not os.path.isabs(link_exe):
            for p in env["ENV"].get("PATH", "").split(os.pathsep):
                candidate = os.path.join(p, link_exe + ".exe")
                if os.path.isfile(candidate):
                    link_exe = candidate
                    break

        cmd_parts = [link_exe]

        # Output target
        cmd_parts.append("-o")
        cmd_parts.append(str(target[0]).replace("\\", "/"))

        # Link flags
        linkflags = env.subst("$LINKFLAGS").strip()
        if linkflags:
            cmd_parts.extend(linkflags.split())

        # RPATH
        rpath = env.subst("$__RPATH").strip()
        if rpath:
            cmd_parts.extend(rpath.split())

        # Use the archive instead of individual .o files.
        # --whole-archive forces ALL members to be included, matching the
        # behavior of listing .o files individually on the command line.
        cmd_parts.append("-Wl,--whole-archive")
        cmd_parts.append(archive_path)
        cmd_parts.append("-Wl,--no-whole-archive")

        # Library directory flags
        libdirflags = env.subst("$_LIBDIRFLAGS").strip()
        if libdirflags:
            libdirflags = libdirflags.replace("\\", "/")
            cmd_parts.extend(libdirflags.split())

        # Library flags - handle paths with spaces
        libflags_raw = env.subst("$_LIBFLAGS").strip()
        if libflags_raw:
            libflags_raw = libflags_raw.replace("\\", "/")
            tokens = libflags_raw.split(" ")
            i = 0
            while i < len(tokens):
                tok = tokens[i]
                if not tok:
                    i += 1
                    continue
                # Merge fragments of multi-word .a paths
                if (not tok.startswith("-") and not tok.startswith('"')
                        and not tok.endswith(".a") and not tok.endswith('.a"')
                        and not tok.startswith("@")):
                    merged = tok
                    j = i + 1
                    found_end = False
                    while j < len(tokens):
                        merged += " " + tokens[j]
                        if tokens[j].endswith(".a") or tokens[j].endswith('.a"'):
                            found_end = True
                            j += 1
                            break
                        j += 1
                    if found_end:
                        cmd_parts.append('"' + merged + '"')
                        i = j
                        continue
                cmd_parts.append(tok)
                i += 1

        # Build the command string
        cmd_str = " ".join(cmd_parts)
        print("  Link command length: %d chars" % len(cmd_str))

        if len(cmd_str) > 30000:
            print("  WARNING: command still long, may fail")

        # Execute via shell to handle quoting properly
        result = subprocess.run(
            cmd_str,
            shell=True,
            cwd=env.Dir("#").abspath,
        )
        return result.returncode

    env.Replace(
        LINKCOM=env.Action(long_link_action, "Linking $TARGET")
    )
