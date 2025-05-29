import { Field, Input, Button, Fieldset, Flex } from "@chakra-ui/react";
import { PasswordInput } from "../components/ui/password-input.tsx";
import { useState } from "react";
import { login } from "../services/auth.ts";
import { useNavigate } from "react-router";
import { toaster } from "../components/ui/toaster.tsx";

function Login() {
  const [username, setUsername] = useState<string>("");
  const [password, setPassword] = useState<string>("");
  const [error, setError] = useState<number>(200);

  const navigate = useNavigate();

  return (
    <Flex w="100%" h="100%" justify="center" align="center">
      <Fieldset.Root
        size="lg"
        maxW="lg"
        p="3.125rem"
        borderWidth="1px"
        invalid={error != 200}
      >
        <Fieldset.Content>
          <Field.Root>
            <Field.Label>Username</Field.Label>
            <Input
              onChange={(event) => {
                setUsername(event.target.value);
              }}
            />
          </Field.Root>

          <Field.Root>
            <Field.Label>Password</Field.Label>
            <PasswordInput
              onChange={(event) => {
                setPassword(event.target.value);
              }}
            />
          </Field.Root>
        </Fieldset.Content>
        <Button
          onClick={async () => {
            const res = await login({ username, password });
            setError(res);
            if (res == 200) {
              toaster.create({
                description: "Login successful",
                type: "success",
              });
              navigate({ pathname: "/" });
            }
          }}
        >
          Login
        </Button>
        {error != 200 && error != 401 && (
          <Fieldset.ErrorText>An error has occurred</Fieldset.ErrorText>
        )}
        {error == 401 && (
          <Fieldset.ErrorText>
            Username or password is incorrect. Please try again
          </Fieldset.ErrorText>
        )}
      </Fieldset.Root>
    </Flex>
  );
}

export default Login;
