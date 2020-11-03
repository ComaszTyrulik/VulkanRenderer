import subprocess
import colorama


class CommandRunner:
    RUNNING_COMMAND_PRINT_TEMPLATE = '{color}Running: {command}\nFrom: {cwd}\n'
    RUNNING_COMMAND_PRINT_COLOR = colorama.Fore.YELLOW

    def run_command(self, command, cwd):
        print(self.RUNNING_COMMAND_PRINT_TEMPLATE.format(color=self.RUNNING_COMMAND_PRINT_COLOR, command=command, cwd=cwd))
        process_result = subprocess.run(args=command, capture_output=True, text=True, cwd=cwd, encoding='utf-8', errors='ignore')

        if self.__does_result_have_stdout(process_result):
            print(colorama.Fore.LIGHTWHITE_EX + process_result.stdout)

        if self.__does_result_have_stderr(process_result):
            print(colorama.Fore.LIGHTRED_EX + process_result.stderr)

        return process_result.returncode

    def __does_result_have_stdout(self, process_result):
        return process_result.stdout is not None and process_result.stdout.__len__() > 0

    def __does_result_have_stderr(self, process_result):
        return process_result.stderr is not None and process_result.stderr.__len__() > 0
