import {Field, Input, Button, Fieldset, Flex} from "@chakra-ui/react";
import {PasswordInput} from "../components/ui/password-input.tsx";

function Login() {
  return (
    <Flex w="100%" h="100%" justify="center" align="center">
      <Fieldset.Root size="lg" maxW="lg" p="3.125rem" borderWidth="1px">
        <Fieldset.Content>
          <Field.Root>
            <Field.Label>Username</Field.Label>
            <Input/>
          </Field.Root>

          <Field.Root>
            <Field.Label>Password</Field.Label>
            <PasswordInput/>
          </Field.Root>
        </Fieldset.Content>
        <Button>Login</Button>
      </Fieldset.Root>
    </Flex>
  );
}

export default Login;
