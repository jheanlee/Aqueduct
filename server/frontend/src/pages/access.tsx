import {
  Alert,
  Button,
  Checkbox,
  Code,
  Collapsible,
  Container,
  Dialog,
  Field,
  Fieldset,
  Flex,
  For,
  Group,
  Heading,
  IconButton,
  Input,
  NumberInput,
  Spacer,
  Table,
  Tabs,
  useClipboard,
} from "@chakra-ui/react";
import { useEffect, useState } from "react";
import {
  checkToken,
  deleteToken,
  listTokens,
  modifyToken,
} from "../services/token";
import { MdDelete, MdEdit } from "react-icons/md";
import { RxPlus } from "react-icons/rx";
import { toaster } from "../components/ui/toaster.tsx";
import { PasswordInput } from "../components/ui/password-input.tsx";
import {
  checkUser,
  deleteUser,
  listUsers,
  modifyUser,
} from "../services/auth.ts";

function Access() {
  return (
    <Container w="100%" h="100%" p="1rem">
      <Tabs.Root variant="subtle">
        <Tabs.List>
          <Tabs.Trigger value="Tokens">Tokens</Tabs.Trigger>
          <Tabs.Trigger value="Web Management">Web Management</Tabs.Trigger>
        </Tabs.List>

        <Tabs.Content value="Tokens">
          <Tokens />
        </Tabs.Content>

        <Tabs.Content value="Web Management">
          <WebManagement />
        </Tabs.Content>
      </Tabs.Root>
    </Container>
  );
}

function Tokens() {
  const [tokensError, setTokensError] = useState<boolean>(false);
  const [tokens, setTokens] = useState<
    {
      name: string;
      token: string;
      notes: string | null;
      expiry: number | null;
    }[]
  >([]);
  const [editToken, setEditToken] = useState<{
    name: string;
    notes: string | null;
  } | null>(null);
  const [deleteTokenName, setDeleteTokenName] = useState<string | null>(null);

  useEffect(() => {
    void fetchData();
  }, []);

  const fetchData = async () => {
    const res = await listTokens();
    if (typeof res === "number") {
      setTokensError(true);
    } else {
      setTokensError(false);
      setTokens(res);
    }
  };

  return (
    <>
      {tokensError && (
        <Alert.Root status="error" m={3}>
          <Alert.Indicator />
          <Alert.Content>
            <Alert.Title>Connection Lost</Alert.Title>
            <Alert.Description>
              Unable to connect to api server
            </Alert.Description>
          </Alert.Content>
        </Alert.Root>
      )}

      <Flex w="100%">
        <Heading>Manage Tokens</Heading>
        <Spacer />
        <NewToken onExitComplete={fetchData} />
      </Flex>

      <Table.Root>
        <Table.Header>
          <Table.Row>
            <Table.ColumnHeader>Name</Table.ColumnHeader>
            <Table.ColumnHeader>Notes</Table.ColumnHeader>
            <Table.ColumnHeader>Expires At</Table.ColumnHeader>
            <Table.ColumnHeader>Actions</Table.ColumnHeader>
          </Table.Row>
        </Table.Header>

        <Table.Body>
          <For each={tokens}>
            {(item) => (
              <Table.Row>
                <Table.Cell>{item.name}</Table.Cell>
                <Table.Cell>{item.notes}</Table.Cell>
                <Table.Cell>
                  {item.expiry !== null
                    ? new Date(item.expiry * 1000)
                        .toISOString()
                        .replace("T", " ")
                        .replace(".000Z", " UTC")
                    : "no expiry"}
                </Table.Cell>
                <Table.Cell>
                  <Group>
                    <IconButton
                      variant="outline"
                      size="sm"
                      onClick={() =>
                        setEditToken({ name: item.name, notes: item.notes })
                      }
                    >
                      <MdEdit />
                    </IconButton>
                    <IconButton
                      variant="outline"
                      size="sm"
                      onClick={() => setDeleteTokenName(item.name)}
                    >
                      <MdDelete />
                    </IconButton>
                  </Group>
                </Table.Cell>
              </Table.Row>
            )}
          </For>
        </Table.Body>
      </Table.Root>

      <EditToken
        name={editToken?.name ?? null}
        notes={editToken?.notes ?? null}
        onExitComplete={async () => {
          await fetchData();
          setEditToken(null);
        }}
      />
      <DeleteToken
        name={deleteTokenName}
        onExitComplete={async () => {
          await fetchData();
          setDeleteTokenName(null);
        }}
      />
    </>
  );
}

interface NewTokenProps {
  onExitComplete: () => void;
}

function NewToken({ onExitComplete }: NewTokenProps) {
  const [nameError, setNameError] = useState<string | null>(null);
  const [newName, setNewName] = useState<string>("");
  const [newNotes, setNewNotes] = useState<string | null>(null);
  const [newExpiry, setNewExpiry] = useState<number>(0);

  const [tokenGenerated, setTokenGenerated] = useState<boolean>(false);
  const [newToken, setNewToken] = useState<string | null>(null);

  const clipboard = useClipboard({ value: newToken || "" });

  const handleExitComplete = () => {
    setNameError(null);
    setNewName("");
    setNewNotes(null);
    setNewExpiry(0);
    setTokenGenerated(false);
    setNewToken(null);

    onExitComplete();
  };

  return (
    <Dialog.Root placement="center" onExitComplete={handleExitComplete}>
      <Dialog.Trigger>
        <IconButton variant="ghost">
          <RxPlus />
        </IconButton>
      </Dialog.Trigger>

      <Dialog.Backdrop />

      <Dialog.Positioner>
        <Dialog.Content>
          <Dialog.Header>
            <Dialog.Title>Create New Token</Dialog.Title>
          </Dialog.Header>

          <Dialog.Body p={5}>
            {!tokenGenerated && (
              <Fieldset.Root>
                <Fieldset.Content>
                  <Field.Root invalid={nameError !== null}>
                    <Field.Label>Name</Field.Label>
                    <Input
                      onChange={async (event) => {
                        setNewName(event.target.value);
                        if (event.target.value == "") {
                          setNameError("required");
                          return;
                        }

                        const res = await checkToken(event.target.value);
                        if (typeof res === "number" || !res.available) {
                          setNameError("unavailable");
                          setNewName("");
                        } else {
                          setNameError(null);
                        }
                      }}
                    />
                    {nameError === "required" && (
                      <Field.ErrorText>This field is required</Field.ErrorText>
                    )}
                    {nameError === "unavailable" && (
                      <Field.ErrorText>
                        Another token with the same name exists
                      </Field.ErrorText>
                    )}
                  </Field.Root>

                  <Field.Root>
                    <Field.Label>Notes</Field.Label>
                    <Input
                      onChange={(event) => {
                        setNewNotes(
                          event.target.value !== "" ? event.target.value : null,
                        );
                      }}
                    />
                  </Field.Root>

                  <Field.Root>
                    <Field.Label>Expiration</Field.Label>
                    <NumberInput.Root
                      defaultValue={"0"}
                      min={0}
                      max={36500}
                      onValueChange={(event) => {
                        setNewExpiry(event.valueAsNumber);
                      }}
                    >
                      <NumberInput.Control />
                      <NumberInput.Input />
                    </NumberInput.Root>
                    <Field.HelperText>
                      Days until expiry (0 for no expiry)
                    </Field.HelperText>
                  </Field.Root>
                </Fieldset.Content>
              </Fieldset.Root>
            )}

            {tokenGenerated && newToken !== null && newToken !== "" && (
              <Flex direction="column" gap={3} m={2}>
                <Alert.Root status="success">
                  <Alert.Indicator />
                  <Alert.Title>New token:</Alert.Title>
                  <Alert.Description>
                    <Code
                      h="100%"
                      size="lg"
                      onClick={() => {
                        clipboard.copy();
                        toaster.create({
                          description: "Token has been copied to clipboard",
                          type: "info",
                        });
                      }}
                    >
                      {newToken}
                    </Code>
                  </Alert.Description>
                </Alert.Root>
              </Flex>
            )}

            {tokenGenerated && newToken === null && (
              <Container>
                <Alert.Root status="error">
                  <Alert.Indicator />
                  <Alert.Title>
                    {"An error has occurred while updating '" + newName + "'"}
                  </Alert.Title>
                </Alert.Root>
              </Container>
            )}
          </Dialog.Body>

          <Dialog.Footer>
            {!tokenGenerated && (
              <Dialog.ActionTrigger>
                <Button variant="outline">Cancel</Button>
              </Dialog.ActionTrigger>
            )}
            {!tokenGenerated && (
              <Button
                onClick={async () => {
                  if (newName == "") {
                    setNameError("required");
                    return;
                  }
                  const check_res = await checkToken(newName);
                  if (typeof check_res === "number" || !check_res.available) {
                    setNameError("unavailable");
                    setNewName("");
                    return;
                  }

                  const res = await modifyToken({
                    name: newName,
                    token_update: true,
                    notes: newNotes != "" ? newNotes : null,
                    expiry_days: newExpiry != 0 ? newExpiry : null,
                  });

                  setTokenGenerated(true);

                  if (typeof res === "number") {
                    setNewToken(null);
                  } else {
                    setNewToken(res.token);
                  }
                }}
              >
                Create
              </Button>
            )}
            {tokenGenerated && (
              <Dialog.ActionTrigger>
                <Button variant="outline">Close</Button>
              </Dialog.ActionTrigger>
            )}
          </Dialog.Footer>
        </Dialog.Content>
      </Dialog.Positioner>
    </Dialog.Root>
  );
}

interface EditTokenProp {
  name: string | null;
  notes: string | null;
  onExitComplete: () => void;
}

function EditToken({ name, notes, onExitComplete }: EditTokenProp) {
  const [newNotes, setNewNotes] = useState<string | null>(notes);
  const [update, setUpdate] = useState<boolean>(false);
  const [newExpiry, setNewExpiry] = useState<number>(0);

  const [tokenGenerated, setTokenGenerated] = useState<boolean>(false);
  const [newToken, setNewToken] = useState<string | null>(null);

  const clipboard = useClipboard({ value: newToken || "" });

  const handleExitComplete = () => {
    setNewNotes(notes);
    setUpdate(false);
    setNewExpiry(0);
    setTokenGenerated(false);
    setNewToken(null);

    onExitComplete();
  };

  return (
    <Dialog.Root
      placement="center"
      open={name !== null}
      onExitComplete={handleExitComplete}
    >
      <Dialog.Backdrop />

      <Dialog.Positioner>
        <Dialog.Content>
          <Dialog.Header>
            <Dialog.Title>Edit '{name}'</Dialog.Title>
          </Dialog.Header>

          <Dialog.Body p={5}>
            {!tokenGenerated && (
              <Fieldset.Root>
                <Fieldset.Content>
                  <Field.Root>
                    <Field.Label>Name</Field.Label>
                    <Input disabled value={name ?? ""} />
                  </Field.Root>

                  <Field.Root>
                    <Field.Label>Notes</Field.Label>
                    <Input
                      value={newNotes !== null ? newNotes : ""}
                      onChange={(event) => {
                        setNewNotes(
                          event.target.value != "" ? event.target.value : null,
                        );
                      }}
                    />
                  </Field.Root>

                  <Checkbox.Root
                    checked={update}
                    onCheckedChange={(event) => {
                      setUpdate(!!event.checked);
                    }}
                  >
                    <Checkbox.HiddenInput />
                    <Checkbox.Control />
                    <Checkbox.Label>Update Token</Checkbox.Label>
                  </Checkbox.Root>

                  <Collapsible.Root open={update}>
                    <Collapsible.Trigger />
                    <Collapsible.Content>
                      <Field.Root>
                        <Field.Label>Expiration</Field.Label>
                        <NumberInput.Root
                          defaultValue={"0"}
                          min={0}
                          max={36500}
                          onValueChange={(event) => {
                            setNewExpiry(event.valueAsNumber);
                          }}
                        >
                          <NumberInput.Control />
                          <NumberInput.Input />
                        </NumberInput.Root>
                        <Field.HelperText>
                          Days until expiry (0 for no expiry)
                        </Field.HelperText>
                      </Field.Root>
                    </Collapsible.Content>
                  </Collapsible.Root>
                </Fieldset.Content>
              </Fieldset.Root>
            )}

            {tokenGenerated && newToken !== null && newToken !== "" && (
              <Alert.Root status="success">
                <Alert.Indicator />
                <Alert.Title>New token:</Alert.Title>
                <Alert.Description>
                  <Code
                    h="100%"
                    size="lg"
                    onClick={() => {
                      clipboard.copy();
                      toaster.create({
                        description: "Token has been copied to clipboard",
                        type: "info",
                      });
                    }}
                  >
                    {newToken}
                  </Code>
                </Alert.Description>
              </Alert.Root>
            )}

            {tokenGenerated && newToken !== null && newToken === "" && (
              <Alert.Root status="success">
                <Alert.Indicator />
                <Alert.Title>
                  {"Notes has been updated for '" + name + "'"}
                </Alert.Title>
              </Alert.Root>
            )}

            {tokenGenerated && newToken === null && (
              <Alert.Root status="error">
                <Alert.Indicator />
                <Alert.Title>
                  {"An error has occurred while updating '" + name + "'"}
                </Alert.Title>
              </Alert.Root>
            )}
          </Dialog.Body>

          <Dialog.Footer>
            {!tokenGenerated && (
              <Button variant="outline" onClick={onExitComplete}>
                Cancel
              </Button>
            )}
            {!tokenGenerated && (
              <Button
                onClick={async () => {
                  if (name === null) {
                    return;
                  }

                  const res = await modifyToken({
                    name: name,
                    token_update: update,
                    notes: newNotes != "" ? newNotes : null,
                    expiry_days: newExpiry != 0 ? newExpiry : null,
                  });

                  setTokenGenerated(true);

                  if (typeof res === "number") {
                    setNewToken(null);
                  } else {
                    setNewToken(res.token);
                  }
                }}
              >
                Update
              </Button>
            )}

            {tokenGenerated && (
              <Button variant="outline" onClick={onExitComplete}>
                Close
              </Button>
            )}
          </Dialog.Footer>
        </Dialog.Content>
      </Dialog.Positioner>
    </Dialog.Root>
  );
}

interface DeleteTokenProp {
  name: string | null;
  onExitComplete: () => void;
}

function DeleteToken({ name, onExitComplete }: DeleteTokenProp) {
  const [flagLoading, setFlagLoading] = useState<boolean>(false);

  return (
    <Dialog.Root
      placement="center"
      role="alertdialog"
      open={name !== null}
      onExitComplete={onExitComplete}
    >
      <Dialog.Backdrop />

      <Dialog.Positioner>
        <Dialog.Content>
          <Dialog.Header>
            <Dialog.Title>Delete '{name}'?</Dialog.Title>
          </Dialog.Header>

          <Dialog.Footer>
            <Button variant="outline" onClick={onExitComplete}>
              Cancel
            </Button>
            <Button
              colorPalette="red"
              loading={flagLoading}
              onClick={async () => {
                if (name === null) {
                  return;
                }

                setFlagLoading(true);

                const res = await deleteToken(name);
                if (typeof res !== "number" && res.rows_affected == 1) {
                  toaster.create({
                    description: "Successfully removed token '" + name + "'",
                    type: "success",
                  });
                } else if (typeof res !== "number" && res.rows_affected == 0) {
                  toaster.create({
                    description:
                      "Couldn't find any token with the name '" + name + "'",
                    type: "info",
                  });
                } else {
                  toaster.create({
                    description:
                      "An error has occurred while deleting token '" +
                      name +
                      "'",
                    type: "error",
                  });
                }

                setFlagLoading(false);

                onExitComplete();
              }}
            >
              Delete
            </Button>
          </Dialog.Footer>
        </Dialog.Content>
      </Dialog.Positioner>
    </Dialog.Root>
  );
}

function WebManagement() {
  const [users, setUsers] = useState<
    {
      username: string;
    }[]
  >([]);
  const [usersError, setUsersError] = useState<boolean>(false);

  const [editUsername, setEditUsername] = useState<string | null>(null);
  const [deleteUsername, setDeleteUsername] = useState<string | null>(null);

  useEffect(() => {
    void fetchData();
  }, []);

  const fetchData = async () => {
    const res = await listUsers();
    if (typeof res === "number") {
      setUsersError(true);
    } else {
      setUsersError(false);
      setUsers(res);
    }
  };

  return (
    <>
      {usersError && (
        <Alert.Root status="error" m={3}>
          <Alert.Indicator />
          <Alert.Content>
            <Alert.Title>Connection Lost</Alert.Title>
            <Alert.Description>
              Unable to connect to api server
            </Alert.Description>
          </Alert.Content>
        </Alert.Root>
      )}

      <Flex w="100%">
        <Heading>Manage Users</Heading>
        <Spacer />
        <NewUser onExitComplete={fetchData} />
      </Flex>

      <Table.Root>
        <Table.Header>
          <Table.Row>
            <Table.ColumnHeader>Username</Table.ColumnHeader>
            <Table.ColumnHeader>Actions</Table.ColumnHeader>
          </Table.Row>
        </Table.Header>

        <Table.Body>
          <For each={users}>
            {(item) => (
              <Table.Row>
                <Table.Cell>{item.username}</Table.Cell>
                <Table.Cell>
                  <Group>
                    <IconButton
                      variant="outline"
                      size="sm"
                      onClick={() => {
                        setEditUsername(item.username);
                      }}
                    >
                      <MdEdit />
                    </IconButton>
                    <IconButton
                      variant="outline"
                      size="sm"
                      onClick={() => {
                        setDeleteUsername(item.username);
                      }}
                    >
                      <MdDelete />
                    </IconButton>
                  </Group>
                </Table.Cell>
              </Table.Row>
            )}
          </For>
        </Table.Body>
      </Table.Root>

      <EditUser
        username={editUsername}
        onExitComplete={async () => {
          await fetchData();
          setEditUsername(null);
        }}
      />
      <DeleteUser
        username={deleteUsername}
        onExitComplete={async () => {
          await fetchData();
          setDeleteUsername(null);
        }}
      />
    </>
  );
}

interface NewUserProp {
  onExitComplete: () => void;
}

function NewUser({ onExitComplete }: NewUserProp) {
  const [open, setOpen] = useState<boolean>(false);
  const [username, setUsername] = useState<string | null>("");
  const [password, setPassword] = useState<string | null>("");
  const [usernameError, setUsernameError] = useState<string | null>(null);
  const [passwordError, setPasswordError] = useState<string | null>(null);

  const handleExitComplete = () => {
    setOpen(false);
    setUsername("");
    setPassword("");
    setUsernameError(null);
    onExitComplete();
  };

  return (
    <Dialog.Root
      placement="center"
      open={open}
      onOpenChange={(event) => setOpen(event.open)}
      onExitComplete={handleExitComplete}
    >
      <Dialog.Trigger>
        <IconButton variant="ghost" size="sm">
          <RxPlus />
        </IconButton>
      </Dialog.Trigger>

      <Dialog.Backdrop />

      <Dialog.Positioner>
        <Dialog.Content>
          <Dialog.Header>
            <Dialog.Title>New User</Dialog.Title>
          </Dialog.Header>

          <Dialog.Body p={5}>
            <Fieldset.Root>
              <Fieldset.Content>
                <Field.Root invalid={usernameError !== null}>
                  <Field.Label>Username</Field.Label>
                  <Input
                    onChange={async (event) => {
                      if (event.target.value === "") {
                        setUsernameError("required");
                        setUsername(null);
                        return;
                      }
                      if (
                        event.target.value.match(
                          RegExp(/^[a-zA-Z][a-zA-Z0-9]{0,31}$/),
                        ) === null
                      ) {
                        setUsernameError("invalid_username");
                        setUsername(null);
                        return;
                      }
                      const res = await checkUser({
                        username: event.target.value,
                      });
                      if (typeof res === "number" || !res.available) {
                        setUsernameError("unavailable");
                        setUsername(null);
                        return;
                      }
                      setUsername(event.target.value);
                      setUsernameError(null);
                    }}
                  />
                  {usernameError === "required" && (
                    <Field.ErrorText>This field is required</Field.ErrorText>
                  )}
                  {usernameError === "unavailable" && (
                    <Field.ErrorText>
                      Another user with the same name exists
                    </Field.ErrorText>
                  )}
                  {usernameError === "invalid_username" && (
                    <Field.ErrorText>Invalid username</Field.ErrorText>
                  )}
                </Field.Root>

                <Field.Root invalid={passwordError !== null}>
                  <Field.Label>Password</Field.Label>
                  <PasswordInput
                    onChange={(event) => {
                      if (
                        event.target.value.match(RegExp(/^[!-~]{1,32}$/)) ===
                        null
                      ) {
                        setPasswordError("invalid_password");
                        setPassword(null);
                        return;
                      }
                      setPassword(event.target.value);
                      setPasswordError(null);
                    }}
                  />
                  {passwordError === "invalid_password" && (
                    <Field.ErrorText>Invalid password</Field.ErrorText>
                  )}
                </Field.Root>
              </Fieldset.Content>
            </Fieldset.Root>
          </Dialog.Body>

          <Dialog.Footer>
            <Dialog.ActionTrigger>
              <Button variant="outline">Cancel</Button>
            </Dialog.ActionTrigger>

            <Button
              onClick={async () => {
                if (username === null || password === null) {
                  return;
                }
                if (username === "") {
                  setUsernameError("required");
                  return;
                }
                if (
                  username.match(RegExp(/^[a-zA-Z][a-zA-Z0-9]{0,31}$/)) === null
                ) {
                  setUsernameError("invalid_username");
                  return;
                }
                if (password.match(RegExp(/^[!-~]{1,32}$/)) === null) {
                  setPasswordError("invalid_password");
                  return;
                }
                const check_res = await checkUser({ username });
                if (typeof check_res === "number" || !check_res.available) {
                  setUsernameError("unavailable");
                  setUsername("");
                  return;
                }

                const res = await modifyUser({ username, password });
                if (res == 200) {
                  toaster.create({
                    description: "Successfully created user",
                    type: "success",
                  });
                } else {
                  toaster.create({
                    description: "An error has occurred while creating user",
                    type: "error",
                  });
                }
                setOpen(false);
              }}
            >
              Create
            </Button>
          </Dialog.Footer>
        </Dialog.Content>
      </Dialog.Positioner>
    </Dialog.Root>
  );
}

interface EditUserProp {
  username: string | null;
  onExitComplete: () => void;
}

function EditUser({ username, onExitComplete }: EditUserProp) {
  const [password, setPassword] = useState<string | null>("");
  const [passwordError, setPasswordError] = useState<string | null>(null);

  const handleExitComplete = () => {
    setPassword("");
    onExitComplete();
  };

  return (
    <Dialog.Root
      placement="center"
      open={username !== null}
      onExitComplete={handleExitComplete}
    >
      <Dialog.Backdrop />

      <Dialog.Positioner>
        <Dialog.Content>
          <Dialog.Header>
            <Dialog.Title>Edit '{username}'</Dialog.Title>
          </Dialog.Header>

          <Dialog.Body p={5}>
            <Fieldset.Root>
              <Fieldset.Content>
                <Field.Root>
                  <Field.Label>Username</Field.Label>
                  <Input disabled value={username || ""} />
                </Field.Root>

                <Field.Root invalid={passwordError !== null}>
                  <Field.Label>Password</Field.Label>
                  <PasswordInput
                    onChange={(event) => {
                      if (
                        event.target.value.match(RegExp(/^[!-~]{1,32}$/)) ===
                        null
                      ) {
                        setPasswordError("invalid_password");
                        setPassword(null);
                        return;
                      }
                      setPasswordError(null);
                      setPassword(event.target.value);
                    }}
                  />
                  {passwordError === "invalid_password" && (
                    <Field.ErrorText>Invalid password</Field.ErrorText>
                  )}
                </Field.Root>
              </Fieldset.Content>
            </Fieldset.Root>
          </Dialog.Body>

          <Dialog.Footer>
            <Button variant="outline" onClick={onExitComplete}>
              Cancel
            </Button>

            <Button
              onClick={async () => {
                if (username === null || password === null) {
                  return;
                }
                if (password.match(RegExp(/^[!-~]{1,32}$/)) === null) {
                  setPasswordError("invalid_password");
                  return;
                }
                const res = await modifyUser({ username, password });
                if (res == 200) {
                  toaster.create({
                    description: "Successfully modified user",
                    type: "success",
                  });
                } else {
                  toaster.create({
                    description: "An error has occurred while modifying user",
                    type: "error",
                  });
                }
                onExitComplete();
              }}
            >
              Update
            </Button>
          </Dialog.Footer>
        </Dialog.Content>
      </Dialog.Positioner>
    </Dialog.Root>
  );
}

interface DeleteUserProp {
  username: string | null;
  onExitComplete: () => void;
}

function DeleteUser({ username, onExitComplete }: DeleteUserProp) {
  const [flagLoading, setFlagLoading] = useState<boolean>(false);

  return (
    <Dialog.Root
      placement="center"
      role="alertdialog"
      open={username !== null}
      onExitComplete={onExitComplete}
    >
      <Dialog.Backdrop />

      <Dialog.Positioner>
        <Dialog.Content>
          <Dialog.Header>
            <Dialog.Title>Delete '{username}'?</Dialog.Title>
          </Dialog.Header>

          <Dialog.Footer>
            <Button variant="outline" onClick={onExitComplete}>
              Cancel
            </Button>
            <Dialog.ActionTrigger>
              <Button
                colorPalette="red"
                loading={flagLoading}
                onClick={async () => {
                  if (username === null) return;
                  setFlagLoading(true);
                  const res = await deleteUser({ username });
                  if (typeof res !== "number" && res.rows_affected == 1) {
                    toaster.create({
                      description:
                        "Successfully removed user '" + username + "'",
                      type: "success",
                    });
                  } else if (
                    typeof res !== "number" &&
                    res.rows_affected == 0
                  ) {
                    toaster.create({
                      description:
                        "Couldn't find any token with the name '" +
                        username +
                        "'",
                      type: "info",
                    });
                  } else {
                    toaster.create({
                      description:
                        "An error has occurred while deleting token '" +
                        username +
                        "'",
                      type: "error",
                    });
                  }
                  setFlagLoading(false);
                  onExitComplete();
                }}
              >
                Delete
              </Button>
            </Dialog.ActionTrigger>
          </Dialog.Footer>
        </Dialog.Content>
      </Dialog.Positioner>
    </Dialog.Root>
  );
}

export default Access;
