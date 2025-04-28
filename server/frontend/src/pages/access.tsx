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
  Spinner,
  Table,
} from "@chakra-ui/react";
import { useEffect, useState } from "react";
import {
  checkToken,
  deleteToken,
  listTokens,
  modifyTokens,
} from "../services/token";
import { MdDelete, MdEdit } from "react-icons/md";
import { RxPlus } from "react-icons/rx";
import { toaster } from "../components/ui/toaster.tsx";

function Access() {
  return (
    <Container w="100%" h="100%" p="1rem">
      <Tokens />
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

  useEffect(() => {
    void fetchData();
  }, []);

  const fetchData = async () => {
    const res = await listTokens();
    if (res !== null) {
      setTokensError(false);
      setTokens(res);
    } else {
      setTokensError(true);
    }
  };

  return (
    <>
      {tokensError && (
        <Alert.Root status="error" m={3}>
          <Alert.Indicator>
            <Spinner size="sm" />
          </Alert.Indicator>
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
                  {(item.expiry !== null) ? new Date(item.expiry * 1000)
                        .toISOString()
                        .replace("T", " ")
                        .replace(".000Z", " UTC")
                    : "no expiry"}
                </Table.Cell>
                <Table.Cell>
                  <Group>
                    <EditToken
                      name={item.name}
                      notes={item.notes}
                      onExitComplete={fetchData}
                    />
                    <DeleteToken name={item.name} onExitComplete={fetchData} />
                  </Group>
                </Table.Cell>
              </Table.Row>
            )}
          </For>
        </Table.Body>
      </Table.Root>
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

                        if (!(await checkToken(event.target.value))) {
                          setNameError("unavailable");
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
                          (event.target.value !== "") ? event.target.value : null,
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
                    <Code size="lg">{newToken}</Code>
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
              <Dialog.CloseTrigger>
                <Button variant="outline">Cancel</Button>
              </Dialog.CloseTrigger>
            )}
            {!tokenGenerated && (
              <Button
                onClick={async () => {
                  if (newName == "") {
                    setNameError("required");
                    return;
                  }
                  if (!(await checkToken(newName))) {
                    setNameError("unavailable");
                    return;
                  }

                  const res = await modifyTokens({
                    name: newName,
                    token_update: true,
                    notes: (newNotes != "") ? newNotes : null,
                    expiry_days: (newExpiry != 0) ? newExpiry : null,
                  });

                  setTokenGenerated(true);

                  if (res !== null) {
                    setNewToken(res.token);
                  } else {
                    setNewToken(null);
                  }
                }}
              >
                Update
              </Button>
            )}
            {tokenGenerated && (
              <Dialog.CloseTrigger>
                <Button variant="outline">Close</Button>
              </Dialog.CloseTrigger>
            )}
          </Dialog.Footer>
        </Dialog.Content>
      </Dialog.Positioner>
    </Dialog.Root>
  );
}

interface EditTokenProp {
  name: string;
  notes: string | null;
  onExitComplete: () => void;
}

function EditToken({ name, notes, onExitComplete }: EditTokenProp) {
  const [newNotes, setNewNotes] = useState<string | null>(notes);
  const [update, setUpdate] = useState<boolean>(false);
  const [newExpiry, setNewExpiry] = useState<number>(0);

  const [tokenGenerated, setTokenGenerated] = useState<boolean>(false);
  const [newToken, setNewToken] = useState<string | null>(null);

  const handleExitComplete = () => {
    setNewNotes(notes);
    setUpdate(false);
    setNewExpiry(0);
    setTokenGenerated(false);
    setNewToken(null);

    onExitComplete();
  };

  return (
    <Dialog.Root placement="center" onExitComplete={handleExitComplete}>
      <Dialog.Trigger>
        <IconButton variant="outline" size="sm">
          <MdEdit />
        </IconButton>
      </Dialog.Trigger>

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
                    <Input disabled value={name} />
                  </Field.Root>

                  <Field.Root>
                    <Field.Label>Notes</Field.Label>
                    <Input
                      value={(newNotes !== null) ? newNotes : ""}
                      onChange={(event) => {
                        setNewNotes(
                          (event.target.value != "") ? event.target.value : null,
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
                  <Code size="lg">{newToken}</Code>
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
              <Dialog.CloseTrigger>
                <Button variant="outline">Cancel</Button>
              </Dialog.CloseTrigger>
            )}
            {!tokenGenerated && (
              <Button
                onClick={async () => {
                  const res = await modifyTokens({
                    name: name,
                    token_update: update,
                    notes: (newNotes != "") ? newNotes : null,
                    expiry_days: (newExpiry != 0) ? newExpiry : null,
                  });

                  setTokenGenerated(true);

                  if (res !== null) {
                    setNewToken(res.token);
                  } else {
                    setNewToken(null);
                  }
                }}
              >
                Update
              </Button>
            )}

            {tokenGenerated && (
              <Dialog.CloseTrigger>
                <Button variant="outline">Close</Button>
              </Dialog.CloseTrigger>
            )}
          </Dialog.Footer>
        </Dialog.Content>
      </Dialog.Positioner>
    </Dialog.Root>
  );
}

interface DeleteTokenProp {
  name: string;
  onExitComplete: () => void;
}

function DeleteToken({ name, onExitComplete }: DeleteTokenProp) {
  const [flagLoading, setFlagLoading] = useState<boolean>(false);

  return (
    <Dialog.Root
      placement="center"
      role="alertdialog"
      onExitComplete={onExitComplete}
    >
      <Dialog.Trigger>
        <IconButton variant="outline" size="sm">
          <MdDelete />
        </IconButton>
      </Dialog.Trigger>

      <Dialog.Backdrop />

      <Dialog.Positioner>
        <Dialog.Content>
          <Dialog.Header>
            <Dialog.Title>Delete '{name}'?</Dialog.Title>
          </Dialog.Header>

          <Dialog.Footer>
            <Dialog.CloseTrigger>
              <Button variant="outline">Cancel</Button>
            </Dialog.CloseTrigger>
            <Dialog.CloseTrigger>
              <Button
                colorPalette="red"
                loading={flagLoading}
                onClick={async () => {
                  setFlagLoading(true);
                  const res = await deleteToken(name);
                  if (res !== null && res.rows_affected == 1) {
                    toaster.create({
                      description: "Successfully removed token '" + name + "'",
                      type: "success",
                    });
                  } else if (res !== null && res.rows_affected == 0) {
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
                }}
              >
                Delete
              </Button>
            </Dialog.CloseTrigger>
          </Dialog.Footer>
        </Dialog.Content>
      </Dialog.Positioner>
    </Dialog.Root>
  );
}

export default Access;
