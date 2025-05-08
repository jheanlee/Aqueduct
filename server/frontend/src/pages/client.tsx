import {
  Alert,
  Collapsible,
  Container,
  createListCollection,
  Field,
  Flex,
  For,
  FormatByte,
  HStack,
  IconButton,
  Input,
  Select,
  Table,
  Tabs,
} from "@chakra-ui/react";
import { IoFilter, IoReload } from "react-icons/io5";
import { useEffect, useState } from "react";
import {
  listClientsDb,
  listConnectedClients,
  updateConnectedClients,
} from "../services/client.ts";

function Client() {
  return (
    <Container w="100%" h="100%" p="1rem">
      <Tabs.Root variant="subtle">
        <Tabs.List>
          <Tabs.Trigger value="Connected">Connected</Tabs.Trigger>
          <Tabs.Trigger value="Usage">All Time Usage</Tabs.Trigger>
        </Tabs.List>

        <Tabs.Content value="Connected">
          <Connected />
        </Tabs.Content>

        <Tabs.Content value="Usage">
          <Usage />
        </Tabs.Content>
      </Tabs.Root>
    </Container>
  );
}

function Connected() {
  const [filterOpen, setFilterOpen] = useState<boolean>(false);
  const [invalidPort, setInvalidPort] = useState<boolean>(false);
  const [clientsError, setClientsError] = useState<boolean>(false);
  const [clients, setClients] = useState<
    {
      key: number;
      ip_addr: string;
      port: number;
      type: number;
      stream_port: number;
      user_addr: string;
      user_port: number;
      main_addr: string;
      main_port: number;
    }[]
  >([]);

  useEffect(() => {
    (async () => {
      const res = await listConnectedClients();
      if (res !== null) {
        setClientsError(false);
        setClients(res);
      } else {
        setClientsError(true);
      }
    })();
  }, []);

  const [filterClient, setFilterClient] = useState<RegExp>(/./);
  const [filterStreamPort, setFilterStreamPort] = useState<RegExp>(/./);
  const clientTypes = createListCollection({
    items: [
      { label: "connection", value: "0" },
      { label: "proxy", value: "4" },
    ],
  });
  const [filterTypes, setFilterTypes] = useState<string[]>([]);
  const [filterUser, setFilterUser] = useState<RegExp>(/./);
  const [filterMain, setFilterMain] = useState<RegExp>(/./);

  return (
    <>
      {clientsError && (
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

      <HStack m="2px">
        <IconButton
          onClick={async () => {
            if (!(await updateConnectedClients())) {
              setClientsError(true);
            }
            const res = await listConnectedClients();
            if (res !== null) {
              setClientsError(false);
              setClients(res);
            } else {
              setClientsError(true);
            }
          }}
          variant="ghost"
          size="sm"
        >
          <IoReload />
        </IconButton>

        <IconButton
          variant="ghost"
          size="sm"
          onClick={() => setFilterOpen(!filterOpen)}
        >
          <IoFilter />
        </IconButton>
      </HStack>

      <Collapsible.Root open={filterOpen}>
        <Collapsible.Trigger />
        <Collapsible.Content>
          <Flex gap={3}>
            <Field.Root>
              <Input
                placeholder="client"
                onChange={(event) =>
                  setFilterClient(event.target.value != "" ? RegExp(event.target.value) : /./)
                }
              />
            </Field.Root>
            <Select.Root
              multiple
              collection={clientTypes}
              onValueChange={(event) => setFilterTypes(event.value)}
            >
              <Select.Control>
                <Select.Trigger>
                  <Select.ValueText placeholder="client type" />
                </Select.Trigger>
                <Select.IndicatorGroup>
                  <Select.Indicator />
                </Select.IndicatorGroup>
              </Select.Control>
              <Select.Positioner>
                <Select.Content>
                  {clientTypes.items.map((type) => (
                    <Select.Item item={type} key={type.value}>
                      {type.label}
                      <Select.ItemIndicator />
                    </Select.Item>
                  ))}
                </Select.Content>
              </Select.Positioner>
            </Select.Root>
            <Field.Root invalid={invalidPort}>
              <Input
                placeholder="stream port"
                onChange={(event) => {
                  setFilterStreamPort(() => {
                    if (event.target.value == "") {
                      setInvalidPort(false);
                      return /./;
                    }
                    if (
                      /^\d*$/.test(event.target.value) &&
                      Number(event.target.value) > 0 &&
                      Number(event.target.value) <= 65535
                    ) {
                      setInvalidPort(false);
                      return RegExp("^" + event.target.value + "\\d*$");
                    }
                    setInvalidPort(true);
                    return /./;
                  });
                }}
              />
              <Field.ErrorText>Invalid port</Field.ErrorText>
            </Field.Root>
            <Field.Root>
              <Input
                placeholder="user"
                onChange={(event) =>
                  setFilterUser(event.target.value != "" ? RegExp(event.target.value) : /./)
                }
              />
            </Field.Root>
            <Field.Root>
              <Input
                placeholder="client main"
                onChange={(event) =>
                  setFilterMain(event.target.value != "" ? RegExp(event.target.value) : /./)
                }
              />
            </Field.Root>
          </Flex>
        </Collapsible.Content>
      </Collapsible.Root>

      <Table.Root>
        <Table.Header>
          <Table.Row>
            <Table.ColumnHeader>Client IP</Table.ColumnHeader>
            <Table.ColumnHeader>Client Type</Table.ColumnHeader>
            <Table.ColumnHeader>Stream Port</Table.ColumnHeader>
            <Table.ColumnHeader>User IP</Table.ColumnHeader>
            <Table.ColumnHeader>Client Main IP</Table.ColumnHeader>
          </Table.Row>
        </Table.Header>

        <Table.Body>
          <For
            each={clients.filter(
              (client) =>
                filterClient.test(client.ip_addr + ":" + client.port) &&
                (filterTypes.length == 0 || filterTypes.includes(client.type.toString())) &&
                filterStreamPort.test(client.stream_port.toString()) &&
                filterUser.test(client.user_addr + ":" + client.user_port) &&
                filterMain.test(client.main_addr + ":" + client.main_port)
            )}
          >
            {(item) => (
              <Table.Row key={item.key}>
                <Table.Cell>{item.ip_addr + ":" + item.port}</Table.Cell>
                <Table.Cell>
                  {(() => {
                    switch (item.type) {
                      case 0:
                        return "connection";
                      case 4:
                        return "proxy";
                      default:
                        return "-";
                    }
                  })()}
                </Table.Cell>
                <Table.Cell>
                  {item.type == 0 ? item.stream_port : "-"}
                </Table.Cell>
                <Table.Cell>
                  {item.type == 4 ? item.user_addr + ":" + item.user_port : "-"}
                </Table.Cell>
                <Table.Cell>
                  {item.type == 4 ? item.main_addr + ":" + item.main_port : "-"}
                </Table.Cell>
              </Table.Row>
            )}
          </For>
        </Table.Body>
      </Table.Root>
    </>
  );
}

function Usage() {
  const [clientsDbError, setClientsDbError] = useState<boolean>(false);
  const [clientsData, setClientsData] = useState<
    {
      ip: string;
      sent: number;
      received: number;
    }[]
  >([]);

  useEffect(() => {
    (async () => {
      const res = await listClientsDb();
      if (res !== null) {
        setClientsDbError(false);
        setClientsData(res);
      } else {
        setClientsDbError(true);
      }
    })();
  }, []);

  const [filterIp, setFilterIp] = useState<RegExp>(/./);

  return (
    <>
      <Alert.Root status="error" m={3} hidden={!clientsDbError}>
        <Alert.Indicator/>
        <Alert.Content>
          <Alert.Title>Connection Lost</Alert.Title>
          <Alert.Description>Unable to connect to api server</Alert.Description>
        </Alert.Content>
      </Alert.Root>

      <Collapsible.Root>
        <HStack m="2px">
          <IconButton
            onClick={async () => {
              if (!(await updateConnectedClients())) {
                setClientsDbError(true);
              }
              const res = await listClientsDb();
              if (res !== null) {
                setClientsDbError(false);
                setClientsData(res);
              } else {
                setClientsDbError(true);
              }
            }}
            variant="ghost"
            size="sm"
          >
            <IoReload />
          </IconButton>

          <Collapsible.Trigger>
            <IconButton variant="ghost" size="sm">
              <IoFilter />
            </IconButton>
          </Collapsible.Trigger>
        </HStack>

        <Collapsible.Content>
          <Input
            placeholder="ip"
            onChange={(event) =>
              setFilterIp(
                event.target.value != "" ? RegExp(event.target.value) : /./,
              )
            }
          />
        </Collapsible.Content>
      </Collapsible.Root>

      <Table.Root>
        <Table.Row>
          <Table.ColumnHeader>IP</Table.ColumnHeader>
          <Table.ColumnHeader>Data Sent</Table.ColumnHeader>
          <Table.ColumnHeader>Data Received</Table.ColumnHeader>
        </Table.Row>
        <Table.Body>
          <For
            each={clientsData.filter((clientData) =>
              filterIp.test(clientData.ip),
            )}
          >
            {(item) => (
              <Table.Row key={item.ip}>
                <Table.Cell>{item.ip}</Table.Cell>
                <Table.Cell>
                  <FormatByte value={item.sent} />
                </Table.Cell>
                <Table.Cell>
                  <FormatByte value={item.received} />
                </Table.Cell>
              </Table.Row>
            )}
          </For>
        </Table.Body>
      </Table.Root>
    </>
  );
}

export default Client;
